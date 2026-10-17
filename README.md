# EventLogger

A C++ application that uses a Raspberry Pi and USB webcam to recognise enrolled faces, track recent presence, and send entry and departure notifications through Telegram.

Events are also written to a local log. Telegram subscribers can register a display name, request the latest event, and unsubscribe. Admin commands provide subscriber management information and direct messaging.

## Features

- Face detection with OpenCV YuNet and feature extraction with SFace.
- Multiple enrollment images per person, matched using cosine similarity.
- Presence tracking with an absence timeout, entry cooldown, and delayed unknown-person alerts.
- Separate threads for camera processing, event consumption, and Telegram polling.
- Local event logs with Singapore time (UTC+8).
- Telegram subscriber names and chat IDs persisted as JSON.
- Graceful shutdown requested with Ctrl+C or SIGTERM.

This is a personal presence-monitoring project. “Entered” and “left” are inferred from camera observations and timeouts; they do not prove that someone physically entered or left the room.

## Requirements

The development setup uses a Raspberry Pi 5, Debian 13 (trixie), GCC 14.2, and a USB webcam supported by V4L2. A display is not required for the main application.

- Linux with V4L2 camera support.
- A C++23-capable compiler and standard library; GCC 14 is sufficient for the current code.
- CMake 3.21 or newer for the provided presets, plus Ninja.
- OpenCV with `core`, `imgcodecs`, `imgproc`, `videoio`, `highgui`, `objdetect`, and `dnn`.
- [tgbot-cpp](https://github.com/reo7sp/tgbot-cpp), Boost, libcurl, and nlohmann/json.
- Internet access and a Telegram bot token.

The following instructions target Debian 13 / Raspberry Pi OS with equivalent packages. Older distributions may provide an insufficient compiler or OpenCV version.

## Setup

### 1. Install dependencies and clone the project

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git curl pkg-config \
    v4l-utils libopencv-dev libboost-dev libcurl4-openssl-dev \
    nlohmann-json3-dev

mkdir -p "$HOME/projects"
cd "$HOME/projects"
git clone https://github.com/RishabhShenoy03/EventLogger.git
```

If the repository is already cloned, keep that checkout and adjust the paths below accordingly.

### 2. Build and install tgbot-cpp

This uses system dependencies and CMake directly, without the upstream Conan-based Makefile. Install into your user directory so no `sudo` is needed for installation.

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

Use a recent tgbot-cpp checkout: this project uses API fields represented by `std::optional`, which differ from older library versions.

### 3. Download the face models

Run from the EventLogger repository root. If these model files are already present and valid, skip the downloads.

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

The models come from [OpenCV Zoo](https://github.com/opencv/opencv_zoo). See its model directories for documentation and model licences.

### 4. Prepare enrollment images

**Before building, update the enrollment path in `include/FaceRegistry.hpp`.** The current source contains an absolute path for the original development machine:

```cpp
const std::filesystem::path enrolled_imgs =
    "/home/rish/Desktop/projects/EventLogger/featuremap_images";
```

For a portable checkout, replace it with:

```cpp
const std::filesystem::path enrolled_imgs{"featuremap_images"};
```

Then always launch the application from the repository root.

Create one directory per enrolled person:

```bash
mkdir -p featuremap_images/Rishabh
```

Place images inside it, for example `featuremap_images/Rishabh/front.jpg` and `featuremap_images/Rishabh/side.jpg`.

The currently recognised directory names are case-sensitive: `Rishabh`, `Daddy`, `Mummy`, `Megan`, and `Shreya`. You only need folders for people you want to enroll. To add different identities, update `PersonID` in `include/person_types.hpp` and both name-conversion functions in `src/person_types.cpp`.

Each image must contain exactly one detected face. Unreadable images, images with zero or multiple detected faces, and images without exactly one extracted feature are skipped. Use clear images with some variation in lighting and angle.

Enrollment extracts reference features at startup; it does not train a new model. Keep your own enrollment images locally—the image directory is ignored by Git.

### 5. Check the camera

```bash
v4l2-ctl --list-devices
v4l2-ctl --device=/dev/video0 --list-formats-ext
```

`src/CameraCapture.cpp` currently opens `/dev/video0` using V4L2 and requests MJPG at 1280×720, 30 FPS. Change these settings if your device uses another path or does not support that mode. The application prints the negotiated resolution at startup.

### 6. Configure and build

```bash
cmake --preset dev -DCMAKE_PREFIX_PATH="$HOME/.local"
cmake --build --preset dev --target event_gateway tele_test -j2
```

`-j2` limits parallel compilation to reduce resource pressure on the Pi. The executables are created under `build/dev/`.

### 7. Configure Telegram

Create a bot through [BotFather](https://t.me/BotFather) and obtain its token. In the repository root, create `.env` with your own values:

```dotenv
TGBOT_API_KEY='YOUR_BOT_TOKEN'
TGBOT_ADMIN_CHAT_ID='YOUR_NUMERIC_PRIVATE_CHAT_ID'
```

Do not put spaces around `=`. Keep `.env` out of Git and do not share the bot token.

```bash
chmod 600 .env
set -a
source .env
set +a
```

The application reads environment variables with `std::getenv`; it does **not** parse `.env` itself. Repeat the export step in each new terminal before running it.

If you do not know your chat ID yet:

1. Set and export the bot token first. `tele_test` does not require the admin ID.
2. Run `./build/dev/tele_test`.
3. Open a private conversation with your bot and send `/start` or a text message.
4. Read the numeric chat ID printed in the terminal.
5. Stop the test with Ctrl+C, replace the admin ID placeholder in `.env`, and export the variables again.

The admin ID is your private conversation's numeric chat ID, not the bot username or the display name chosen during registration. Do not run `tele_test` and `event_gateway` simultaneously with the same bot token.

### 8. Run

From the repository root:

```bash
mkdir -p outputs
./build/dev/event_gateway
```

Creating `outputs` is necessary: the current main program opens the log file but does not create its parent directory.

At startup, the application opens the camera, loads the models, and enrolls the reference images. Face counts printed before `Enroll done` refer to enrollment images.

In Telegram, send `/start`, then send your chosen display name as a separate message. Once registered, you receive future presence notifications. Events that happened before registration are not replayed; use `/latest` to query the latest event in the current run.

Press Ctrl+C to request shutdown. Active camera or network operations may take time to finish before the process exits.

## Telegram commands

| Command | Purpose | Access |
| --- | --- | --- |
| `/start` | Register or update your display name; send the name in the next message | Any chat |
| `/stop` | Unsubscribe and cancel pending name registration | Any chat |
| `/latest` | Show the latest logged event in the current process | Any chat |
| `/admin_sublist` | List subscriber display names and chat IDs | Configured admin chat |
| `/admin_subcount` | Show the subscriber count | Configured admin chat |
| `/admin_message <chat_id> <text>` | Send a message to an existing subscriber | Configured admin chat |

Example admin message:

```text
/admin_message 123456789 Hello from the Pi!
```

Use single spaces between the command and recipient ID with the current parser. The remaining text is the message body.

Registration is currently open, and `/latest` is not restricted to subscribers or the admin. Use the bot with trusted users; display names do not grant admin access.

## Configuration and stored files

| Setting | Current value | Location |
| --- | --- | --- |
| Recognition threshold | Cosine similarity greater than 0.6 | `src/FaceRegistry.cpp` |
| Absence timeout | 30 seconds | `include/PresenceTracker.hpp` |
| Global entry cooldown | 12 seconds | `include/PresenceTracker.hpp` |
| Unknown confirmation delay | 3 seconds | `include/PresenceTracker.hpp` |
| Camera mode | `/dev/video0`, MJPG, 1280×720, 30 FPS requested | `src/CameraCapture.cpp` |
| Log timestamp display | SGT, fixed UTC+8 offset | `src/Logger.cpp` |

Changes to these source settings require a rebuild. Similarity is a feature-comparison score, not a probability that an identity is correct.
