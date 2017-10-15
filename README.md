# BluetoothLightSwitch
This project includes code and a schematic to make a bluetooth opperated light switch.
It accomplishes this using the Arduino Nano and HM-10

Because the HM-10 I had was actually a CC41-A, the "BRK" pin doesn't exist.
This lead me to connect the bluetooth module's VCC to an output on the Arduino so that it could be power cycled in order to disconnect clients
