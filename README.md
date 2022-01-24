# BleDistance
Distance tracking for BLE iBeacons in esphome

This code utilizes esphome's esp32_ble_tracker component to track nearby iBeacons. The distance is estimated using the received RSSI and the beacon's advertised 1 meter reference RSSI. The distance is then filtered similarly as in the ESPresense project before being reported.
