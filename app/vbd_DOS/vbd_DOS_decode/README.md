## Build on ubuntu
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
sudo apt-get install libglfw3
sudo apt-get install libglfw3-dev
sudo apt-get install libfftw3-3
sudo apt-get install libfftw3-de

g++ decode_wav.cpp -o decode_wav -lSDL2

## Run catch up sound 44100Hz sample rate
## Prepare mic and sound input to decode (ex play ViVi68686868.wav and input to microphone)
decode_wav


## Sound Tx/Rx

The data communicated through sound contains the contact information required to initialize the WebRTC connection. This data is stored in the [Session Description Protocol (SDP)](https://en.wikipedia.org/wiki/Session_Description_Protocol) format. Since data-over-sound has significant limitations in terms of bandwidth and robustness it is desirable to transmit as few data as possible. Therefore, the SDP is stripped from all irrelevant information and only the essential data needed to establish the connection is transmitted. Currently, the sound packet containing the minimum required SDP data has the following format:

| Size, [B] | Description |
| --------- | ----------- |
| 1         | Type of the SDP - Offer or Answer |
| 1         | Packet size in bytes (not including ECC bytes) |
| 4         | IP address of the transmitting peer |
| 2         | Network port that will be used for the communication |
| 32        | SHA-256 fingerprint of the session data |
| 40        | ICE Credentials - 16 bytes username + 24 bytes password |
| 32        | ECC correction bytes used to correct errors during Tx |

The total size of the audio packet is 112 bytes. With the current audio encoding algorithm, the SDP packet can be transmitted in 5-10 seconds (depending on the Tx protocol used). Using slower protocols provides more reliable transmission in noisy environments or if the communicating devices are far from each other.

### Data-to-sound encoding

The current approach uses a multi-frequency [Frequency-Shift Keying (FSK)](https://en.wikipedia.org/wiki/Frequency-shift_keying) modulation scheme. The data to be transmitted is first split into 4-bit chunks. At each moment of time, 3 bytes are transmitted using 6 tones - one tone for each 4-bit chunk. The 6 tones are emitted in a 4.5kHz range divided in 96 equally-spaced frequencies:

| Freq, [Hz]   | Value, [bits]   | Freq, [Hz]   | Value, [bits]   | ... | Freq, [Hz]   | Value, [bits]   |
| ------------ | --------------- | ------------ | --------------- | --- | ------------ | --------------- |
| `F0 + 00*dF` | Chunk 0: `0000` | `F0 + 16*dF` | Chunk 1: `0000` | ... | `F0 + 80*dF` | Chunk 5: `0000` |
| `F0 + 01*dF` | Chunk 0: `0001` | `F0 + 17*dF` | Chunk 1: `0001` | ... | `F0 + 81*dF` | Chunk 5: `0001` |
| `F0 + 02*dF` | Chunk 0: `0010` | `F0 + 18*dF` | Chunk 1: `0010` | ... | `F0 + 82*dF` | Chunk 5: `0010` |
| ...          | ...             | ...          | ...             | ... | ...          | ...             |
| `F0 + 14*dF` | Chunk 0: `1110` | `F0 + 30*dF` | Chunk 1: `1110` | ... | `F0 + 94*dF` | Chunk 5: `1110` |
| `F0 + 15*dF` | Chunk 0: `1111` | `F0 + 31*dF` | Chunk 1: `1111` | ... | `F0 + 95*dF` | Chunk 5: `1111` |

For all protocols: `dF = 46.875 Hz`. For non-ultrasonic protocols: `F0 = 1875.000 Hz`. For ultrasonic protocols: `F0 = 15000.000 Hz`.

## Known problems / stuff to improve

  - Does not work with: IE, IE Edge, Chrome/Firefox on iOS, Safari on macOS
  - Ultrasonic sound transmission does not work on most devices. Probably hardware limitations?
  - In presence of multiple local networks, cannot currently select which one to use. Always the first one is used
  - There is occasionally sound cracking during transmission. Need to optimize the Tx code
  - The size of the emscripten generated .js is too big (~1MB). Rewrite in pure JS?
  - On mobile, using Firefox, the page can remain running in the background even after closing the tab
