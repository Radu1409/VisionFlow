```bash
sudo apt install ffmpeg
```
 
### 2. Install v4l2loopback (virtual camera driver)
 
The apt version is incompatible with kernel 6.8.x — install from source:
 
```bash
sudo apt install linux-headers-$(uname -r) git build-essential
pushd ~/Documents/
git clone https://github.com/umlaeute/v4l2loopback.git ~/Documents/v4l2loopback
cd ~/Documents/v4l2loopback
make
sudo make install
```
 
---
 
## Running the test client
 
### Step 1 — Load the virtual camera driver
 
```bash
sudo modprobe v4l2loopback
```
 
Verify that `/dev/video0` exists:
 
```bash
ls /dev/video*
```
 
> **Note:** The driver must be reloaded after every reboot:
> ```bash
> sudo modprobe v4l2loopback
> ```
 
### Step 2 — Start the frame source (keep this terminal open)
 
```bash
ffmpeg -f lavfi -i testsrc=size=640x480:rate=30 -pix_fmt yuyv422 -f v4l2 /dev/video0
```
 
### Step 3 — Build the test client (separate terminal)
 
```bash
cd test_clients/vf_camera/build
make
```
 
### Step 4 — Run the test client
 
```bash
cd test_clients/vf_camera/build
./test_vf_camera
```
 
Captured frames are saved to `test_clients/vf_camera/build/frames_out/`.
 
---
 
## Viewing captured frames
 
```bash
ffplay -f rawvideo -pixel_format yuyv422 -video_size 640x480 frames_out/frame_000_640x480.raw
```
 
---
 
## Cleaning captured frames
 
```bash
./test_vf_camera clean
```
 
---
 
## Output example
 
```
frames_out/
├── frame_000_640x480.raw
├── frame_001_640x480.raw
├── ...
└── frame_009_640x480.raw
```
 
