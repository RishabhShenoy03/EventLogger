# EventLogger

A C++ application that uses a Raspberry Pi and USB webcam to recognise enrolled faces, track recent presence, and send entry and departure notifications through Telegram.

Events are also written to a local log. Telegram subscribers can register a display name, request the latest event, contact the admin, and unsubscribe. Admin commands support subscriber listing, private messaging, and broadcasts.

## Features

- Face detection with OpenCV YuNet and feature extraction with SFace.
- Multiple enrollment images per person, matched using cosine similarity.
- Presence tracking with an absence timeout, entry cooldown, and delayed unknown-person alerts.
- Separate threads for camera processing, event consumption, and Telegram polling.
- Local event logs with Singapore time (UTC+8).
- Telegram subscriber names and chat IDs persisted as JSON.
- Admin-only subscriber information, private messages, and broadcasts.
- Shutdown requested with Ctrl+C or SIGTERM.

“Entered” and “left” are inferred from camera observations and timeouts. They do not prove that someone physically entered or left the room.

## Requirements

The development setup uses a Raspberry Pi 5, Debian 13 (trixie), GCC 14.2, and a USB webcam supported by V4L2.

- Linux with V4L2 camera support.
- A C++23-capable compiler and standard library.
- CMake 3.21 or newer for the provided presets.
- Ninja.
- OpenCV with `core`, `imgcodecs`, `imgproc`, `videoio`, `highgui`, `objdetect`, and `dnn`.
- [tgbot-cpp](https://github.com/reo7sp/tgbot-cpp).
- Boost, libcurl, and nlohmann/json.
- Internet access and a Telegram bot token.

GCC 14 is sufficient for the current code. A display is not required for the main application.

The instructions below target Debian 13 or a Raspberry Pi OS installation with equivalent packages.

## Setup

### 1. Install dependencies

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git curl pkg-config \
    v4l-utils libopencv-dev libboost-dev libcurl4-openssl-dev \
    nlohmann-json3-dev
```

### 2. Clone the project

```bash
mkdir -p "$HOME/projects"
cd "$HOME/projects"

git clone https://github.com/RishabhShenoy03/EventLogger.git
```

If the repository is already cloned, use that checkout and adjust the paths below.

### 3. Build and install tgbot-cpp

These commands use system dependencies and CMake directly, without the upstream Conan-based Makefile.

```bash
cd "$HOME/projects"
git clone https://github.com/reo7sp/tgbot-cpp.git

cmake -S tgbot-cpp -B tgbot-cpp/build/local -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$HOME/.local" \
    -DENABLE_TESTS=OFF

cmake --build tgbot-cpp/build/local -j2
cmake --install tgbot-cpp/build/local
```

The library is installed into your user directory, so installation does not require `sudo`.

Use a recent tgbot-cpp checkout. This project uses API fields represented by `std::optional`, which differ from older library versions.

### 4. Download the face models

Run from the EventLogger repository root. Skip this step if valid copies of both model files are already present.

```bash
cd "$HOME/projects/EventLogger"
mkdir -p models

curl -fL --retry 3 \
    https://media.githubusercontent.com/media/opencv/opencv_zoo/main/models/face_detection_yunet/face_detection_yunet_2023mar.onnx \
    -o models/face_detection_yunet_2023mar.onnx

curl -fL --retry 3 \
    https://media.githubusercontent.com/media/opencv/opencv_zoo/main/models/face_recognition_sface/face_recognition_sface_2021dec.onnx \
    -o models/face_recognition_sface_2021dec.onnx
```

The models come from [OpenCV Zoo](https://github.com/opencv/opencv_zoo). See the corresponding model directories for documentation and licences.

### 5. Prepare enrollment images

Check the enrollment path in `include/FaceRegistry.hpp`. If it still points to the original development machine:

```cpp
const std::filesystem::path enrolled_imgs =
    "/home/rish/Desktop/projects/EventLogger/featuremap_images";
```

Replace it with a path appropriate for your installation. To use a directory relative to the repository root:

```cpp
const std::filesystem::path enrolled_imgs{"featuremap_images"};
```

Always launch the application from the repository root when using relative paths.

Create one folder per enrolled person:

```bash
mkdir -p featuremap_images/Rishabh
```

Place reference images inside that folder, for example:

```text
featuremap_images/Rishabh/front.jpg
featuremap_images/Rishabh/side.jpg
```

The currently supported folder names are case-sensitive:

- `Rishabh`
- `Daddy`
- `Mummy`
- `Megan`
- `Shreya`

Only create folders for the people you want to enroll.

To add another identity, update `PersonID` in `include/person_types.hpp` and both name-conversion functions in `src/person_types.cpp`.

Each image must contain exactly one detected face. The application skips:

- Unreadable images.
- Images with zero or multiple detected faces.
- Images that do not produce exactly one extracted feature.

Use clear reference images with some variation in lighting and angle.

Enrollment extracts reference features at startup; it does not train a new model. Enrollment images are local data and are ignored by Git.

### 6. Check the camera

List available devices and supported modes:

```bash
v4l2-ctl --list-devices
v4l2-ctl --device=/dev/video0 --list-formats-ext
```

`src/CameraCapture.cpp` requests:

| Setting | Value |
| --- | --- |
| Device | `/dev/video0` |
| Backend | V4L2 |
| Format | MJPG |
| Resolution | 1280×720 |
| Frame rate | 30 FPS |

Change the settings if your camera uses a different device path or does not support that mode.

The application prints the negotiated resolution at startup.

### 7. Configure and build

From the repository root:

```bash
cmake --preset dev -DCMAKE_PREFIX_PATH="$HOME/.local"
cmake --build --preset dev --target event_gateway tele_test -j2
```

Executables are created under `build/dev/`.

`-j2` limits parallel compilation to reduce resource pressure on the Pi.

### 8. Configure Telegram

Create a bot through [BotFather](https://t.me/BotFather) and obtain its API token.

Create `.env` in the repository root:

```dotenv
TGBOT_API_KEY='YOUR_BOT_TOKEN'
TGBOT_ADMIN_CHAT_ID='YOUR_NUMERIC_PRIVATE_CHAT_ID'
```

Do not put spaces around `=`. Keep the token private and do not commit `.env`.

Export the variables into the current terminal:

```bash
chmod 600 .env

set -a
source .env
set +a
```

The application reads environment variables using `std::getenv`; it does not load `.env` automatically. Repeat the export step in each new terminal before running the application.

#### Finding your admin chat ID

1. Set and export the bot token first. The Telegram test does not require the admin ID.
2. Run:

   ```bash
   ./build/dev/tele_test
   ```

3. Open a private conversation with your bot.
4. Send `/start` or a text message.
5. Read the numeric chat ID printed in the terminal.
6. Stop the test with Ctrl+C.
7. Replace the admin ID placeholder in `.env` and export the variables again.

The admin ID is your private conversation's numeric chat ID. It is not the bot username or the display name chosen during registration.

Do not run `tele_test` and `event_gateway` simultaneously with the same bot token.

### 9. Run

From the repository root:

```bash
mkdir -p outputs
./build/dev/event_gateway
```

The `outputs` directory must exist before the application opens its log file.

At startup, the application opens the camera, loads the models, and enrolls the reference images. Face counts printed before `Enroll done` refer to enrollment images.

In Telegram, send `/start`, followed by your display name in a separate message. You will then receive future presence notifications.

Press Ctrl+C to request shutdown. Active camera or network operations may take time to finish before the process exits.

## Telegram commands

### User commands

| Command | Purpose |
| --- | --- |
| `/help` | Show the command reference |
| `/start` | Register for notifications or update your display name |
| `/stop` | Unsubscribe and cancel pending name registration |
| `/latest` | Show the latest logged event from the current application run |
| `/contact <text>` | Send a message to the admin |

#### Registering

Send:

```text
/start
```

Then send your chosen display name as a separate message. After registration or a name update, the bot sends a confirmation and the command reference. You can request the reference again with `/help`.

The reference includes admin commands, but displaying them does not grant permission to use them.

Registered subscribers receive future presence notifications. Previous notifications are not replayed.

Send `/start` again to change your display name. Subscriber names and chat IDs are saved in `data/subscribers.json` and restored at startup.

Send `/stop` to unsubscribe or cancel pending registration.

#### Contacting the admin

```text
/contact Hello! Could you check the camera?
```

The admin receives your message with your display name and numeric chat ID. Unregistered senders are labelled `Unregistered User`.

Ordinary text messages are not automatically forwarded. Use `/contact` when messaging the admin.

### Admin commands

Admin commands require the sender's chat ID to match `TGBOT_ADMIN_CHAT_ID`. Display names do not grant admin permissions.

| Command | Purpose |
| --- | --- |
| `/subs` | List subscriber display names and chat IDs |
| `/count` | Show the number of subscribers |
| `/msg <chat_id> <text>` | Send a private message to a registered subscriber |
| `/broadcast <text>` | Send an announcement to subscribers, excluding the admin chat |

List subscribers:

```text
/subs
```

Check the subscriber count:

```text
/count
```

Send a private message:

```text
/msg 123456789 Hello from the Pi!
```

Replace `123456789` with a numeric chat ID from `/subs`. The recipient must already be subscribed.

Use a single space between the command, recipient ID, and message text.

Send an announcement:

```text
/broadcast The camera will be offline for maintenance tonight.
```

Recipients see messages labelled as coming from the admin. Keep broadcast text on one line with the current parser.

Broadcasts attempt delivery to every subscriber except the admin chat. A failure for one recipient does not stop attempts to the remaining recipients. The admin receives a success or partial-failure summary after the attempts finish. Failed deliveries are not automatically retried.

The current success message means no send attempt reported an error; it does not confirm that recipients have read the message. If there are no eligible recipients, the bot still reports success.

### Access behaviour

- Registration is open.
- `/help`, `/latest`, and `/contact` are available to unregistered users.
- `/subs`, `/count`, `/msg`, and `/broadcast` are admin-only.
- The admin receives a notification when someone registers or updates their display name.
- `/latest` refers to the current application run; previous events are not reloaded from the log file.

## Configuration

| Setting | Current value | Location |
| --- | --- | --- |
| Recognition threshold | Cosine similarity greater than 0.6 | `src/FaceRegistry.cpp` |
| Absence timeout | 10 minutes | `include/PresenceTracker.hpp` |
| Global entry cooldown | 30 seconds | `include/PresenceTracker.hpp` |
| Unknown confirmation delay | 5 seconds | `include/PresenceTracker.hpp` |
| Camera mode | `/dev/video0`, MJPG, 1280×720, 30 FPS requested | `src/CameraCapture.cpp` |
| Log timestamp display | SGT, fixed UTC+8 offset | `src/Logger.cpp` |

Changes to source settings require a rebuild.

Similarity is a feature-comparison score, not a probability that an identity is correct.

## Stored files

| Path | Contents |
| --- | --- |
| `models/` | YuNet and SFace ONNX models |
| `featuremap_images/<person>/` | Enrollment images |
| `outputs/events.log` | Appended event log |
| `data/subscribers.json` | Subscriber chat IDs and display names |
| `.env` | Bot token and admin chat ID |

Subscriber changes are written to a temporary JSON file before replacing the destination.

Subscribers persist across restarts. Presence state and latest-event information are held in memory and reset when the application restarts.

## Architecture

| Component | Responsibility |
| --- | --- |
| `CameraCapture` | Capture webcam frames |
| `FaceProcessor` | Detect faces, extract features, and compare similarity |
| `FaceRegistry` | Enroll reference images and select the best identity match |
| `PresenceTracker` | Convert matches into entry and departure events |
| `EventQueue` | Transfer events between producer and consumer threads |
| `Logger` | Format events, write logs, and expose latest-event information |
| `TeleNotifier` | Handle Telegram commands, subscribers, and notifications |

The producer captures frames, recognises faces, and queues presence changes.

The consumer logs each event and sends Telegram notifications.

A separate Telegram thread handles commands through long polling. No public webhook endpoint or port forwarding is required.

## Manual testing

The automated test targets are currently commented out in `CMakeLists.txt`. The manual `tele_test` and `camera_capture_test` executables are available, but there is no enabled CTest suite.

A useful smoke test is:

1. Register with `/start` and verify your name appears in `/subs`.
2. Start outside the camera view, then enter after registration.
3. Check that an entry appears in the local log and Telegram.
4. Stay visible and check that entry events are not logged continuously.
5. Leave the camera view for longer than the 10-minute absence timeout and check for a departure event.
6. Test `/help`, `/latest`, `/count`, `/contact`, and `/msg`, and verify that a non-admin cannot execute admin commands.
7. Test `/broadcast` with consenting test subscribers.
8. Restart and verify that subscribers remain registered.
9. Use `/stop` and verify that the chat stops receiving future notifications.

## Troubleshooting

### CMake cannot find TgBot

Complete the tgbot-cpp installation, then configure again:

```bash
cmake --preset dev -U TgBot_DIR -DCMAKE_PREFIX_PATH="$HOME/.local"
```

### Camera cannot be opened

Check the device path, permissions, supported formats, and whether another process is using it:

```bash
v4l2-ctl --list-devices
v4l2-ctl --device=/dev/video0 --list-formats-ext
fuser /dev/video0
ls -l /dev/video0
```

### Models or enrollment files cannot be found

Run from the repository root and check the enrollment path in `FaceRegistry.hpp`.

Ensure the ONNX files contain the actual models rather than Git LFS pointer text.

### Environment variable is missing

Export `.env` in the terminal used to launch the application:

```bash
set -a
source .env
set +a
```

The token variable is `TGBOT_API_KEY`, even if the error message refers to `TGBOT_BOT_TOKEN`.

### Telegram notifications are missing

Complete name registration, confirm the chat appears in `/subs`, and trigger a new event.

Check the local log and terminal for errors. Polling retries after network errors, but failed outgoing notifications are not replayed automatically.

### No log file appears

Create `outputs/`, confirm it is writable, and run from the repository root.

### Subscriber loading fails

Check `data/subscribers.json` for malformed JSON. Back up the file before repairing it.

## Current limitations

- Recognition depends on face visibility, lighting, camera quality, and enrollment images.
- Turning away can result in an unknown match or an absence timeout.
- Unknown detections share one `Unknown` identity; separate unknown people are not tracked individually.
- The global entry cooldown can delay or suppress notifications when people arrive close together.
- Camera failures currently have no automatic reconnection path.
- Telegram delivery runs in the event-consumer thread, so slow network requests can delay subsequent event processing.
- Registration is open, and `/latest` is not restricted to subscribers.
- The application currently requires manual startup.
