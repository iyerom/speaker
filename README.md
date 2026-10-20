# speaker
A small IoT device that reads messages out loud. Anyone with the link opens a web page, types a name and a line of text, and hits send. This updates a realtime database which is then streamed from an esp32 sitting on my desk, and said out loud via a speaker.  

[video] 


## Hardware
  
| Part | Purpose |
|---|---|
| ESP32 (prototyped on a WROOM devkit, designed to move to an ESP32-S3 Supermini) | WiFi, Firebase client, audio decoding |
| MAX98357A I2S amplifier breakout | Digital to Analog and Amplifier in one, driven by I2S |
| 4 Ω 3 W speaker | Output |
| 5 V USB power adapter | The device is stationary and wall powered, so no battery circuit |

## Known limitations and next steps
 
- A new message interrupts the one currently being spoken
- The speech uses the Google Translate TTS endpoint built into the audio library, which has a time limit per prompt
- No rate limiting yet
- Sending the exact same line twice in a row does not retrigger the device as the database value does not change
