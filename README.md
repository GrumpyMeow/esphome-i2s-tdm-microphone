# ESPHome i2s TDM microphone external component
External ESPHome component for multi-lane TDM microphone support

## Goal
The goal for this external component is to support the [Sipeed 6+1 Microphone Array](https://wiki.sipeed.com/hardware/en/modules/micarray.html) in ESPHome using Multi-Lane i2s on the ESP32P4.
The current ESPHome i2s_audio implementation is created for ease-of-use which probroably won't benefit of the complexity of extending it to support TDM.
The external component will probably provide an `i2s_tdm_audio` compponent and `i2s_tdm_microphone` component.


## Other options
The microphone array probably can also be connected to the ESP32P4 using the [MA-USB8 board](https://wiki.sipeed.com/hardware/en/modules/micarray_usbboard_bl616.html) and then wait for this [Pull Request](https://github.com/esphome/esphome/pull/11728) to be completed.




## Todo
* [ ] Create the needed folder structure and files to allow for importing the external-component.
* [ ] Implement the skeleton `i2s_tdm_audio` component
* [ ] Implement the skeleton `i2s_tdm_microphone` component
* [ ] Create the bare-minimum of configuration schema
* [ ] Implement the `i2s_tdm_audio` component
* [ ] Implement the `i2s_tdm_microphone` componet
