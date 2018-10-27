#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <string>
#include <vector>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

namespace hapi
{
class USBCamera
{
  public:
    enum TriggerType
    {
        SOFTWARE,
        HARDWARE
    };

    static void cleanup();
    static unsigned int num_cams();
    static std::shared_ptr<Camera> get(unsigned int id);

    void configure_trigger(TriggerType type);
    void grab_next_image_by_trigger();
    void reset_trigger();
    void print_device_info();
    void set_aquisition_mode(gcstring mode="Continuous");
    void begin_acquisition();
    ImagePtr acquire_image();
    void end_acquisition();
    void init();
    void deinit();

  private:
    USBCamera();
    ~USBCamera();
    
    CameraPtr _ptr;
    TriggerType _type{TriggerType::SOFTWARE};

    static void init_sys();

    static std::shared_ptr<SystemPtr> _system;
    static std::shared_ptr<CameraList> _clist;
    static std::vector<std::shared_ptr<Camera>> _cameras;
};
}
#endif