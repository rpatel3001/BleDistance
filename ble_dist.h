#ifndef BLE_DIST
#define BLE_DIST

#define TAG "ble_dist"
#define TIMEOUT 120
#define BUF_SIZE 9
#define MAX_DIST NAN
#define MIN_RSSI NAN

#define FCMIN 0.0001
#define BETA 0.05
#define DCUTOFF 1.0

#define PATH_LOSS 2.7

class BeaconTracker {

  public:
    std::string name;
    esphome::esp32_ble_tracker::ESPBTUUID uuid;

  private:
    OneEuro filter;
    std::vector<float> dist_buf;
    std::vector<time_t> time_buf;
    time_t now;
    int rssi;
    float filt_dist;

  public:
    BeaconTracker(std::string n, std::string u, float fcmin, float beta)
    : name(n)
    , uuid(esphome::esp32_ble_tracker::ESPBTUUID::from_raw(u.c_str()))
    , filter(1.0, fcmin, beta, DCUTOFF)
    {
      dist_buf.reserve(BUF_SIZE+1);
      time_buf.reserve(BUF_SIZE+1);
    }

  private:
    void validate() {
      now = esp_timer_get_time();
      while(!time_buf.empty() && (time_buf.size() > BUF_SIZE || now - time_buf.back() > TIMEOUT * 1e6)) {
        dist_buf.pop_back();
        time_buf.pop_back();
      }
    }

  public:
    void update(int r, int p) {
      now = esp_timer_get_time();
      rssi = r;
      dist_buf.insert(dist_buf.begin(), 3.281 * pow(10.0, (p-r)/(10.0 * PATH_LOSS)));
      time_buf.insert(time_buf.begin(), now);
      validate();
      auto temp = dist_buf;
      std::sort(temp.begin(), temp.end());
      filt_dist = filter(temp[std::max(0u,(unsigned)(dist_buf.size()/3.0-1))], now);
      std::string s = "";
      for(auto b : temp)
        s += esphome::to_string(b) + ", ";
      ESP_LOGD(TAG, "Recognized %s iBeacon: %s", name.c_str(), uuid.to_string().c_str());
      ESP_LOGD(TAG, "  RSSI: %d", r);
    }

    float get_rssi() {
      validate();
      return time_buf.empty() ? MIN_RSSI : std::max((float)rssi, MIN_RSSI);
    }

    float get_raw_dist() {
      validate();
      return time_buf.empty() ? MAX_DIST : std::min(dist_buf[0], MAX_DIST);
    }

    float get_dist() {
      validate();
      return time_buf.empty() ? MAX_DIST : std::min(filt_dist, MAX_DIST);
    }

};

std::vector<BeaconTracker> trackers;

void parseAdvertisement(esphome::esp32_ble_tracker::ESPBTDevice dev) {
  if(dev.get_ibeacon().has_value()) {
    auto ib = dev.get_ibeacon().value();
    auto uuid = ib.get_uuid();
    auto txpwr = ib.get_signal_power();
    auto rssi = dev.get_rssi();
    for(auto &t : trackers)
      if(t.uuid == uuid)
        t.update(rssi, txpwr);
  }
}

void addTracker(std::string n, std::string u, float fcmin = FCMIN, float beta = BETA) {
  trackers.emplace(trackers.end(), n, u, fcmin, beta);
}

BeaconTracker& getTracker(std::string n) {
  for(auto &t : trackers)
    if(t.name == n)
      return t;
  ESP_LOGW(TAG, "BeaconTracker %s not recognized, returning the first tracker (hopefully one exists)", n.c_str());
  return trackers[0];
}

#endif
