#include <yarp/os/all.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Image.h>
#include <iCub/eventdriven/all.h>
#include <iCub/ctrl/adaptWinPolyEstimator.h>
using namespace ev;

/******************************************************************************/
//eventGatePort
/******************************************************************************/
//This class is an input port with a corresponding output port.
//Default operation is to send the input to the output
//By calling disactivate the input will be discarded instead
class eventGatePort : public yarp::os::BufferedPort<vBottle>
{
private:

    //gate state
    bool active;

    //output port (also this class has a port of it's own)
    yarp::os::BufferedPort<vBottle> output_port;

public:

    eventGatePort() : active(true) {}
    ~eventGatePort();

    void activate();
    void disactivate();

    virtual void interrupt();
    virtual void close();
    virtual void onRead(vBottle &input);
    virtual bool open(const yarp::os::ConstString &name);

};

/******************************************************************************/
//saccadicSuppression
/******************************************************************************/
//This is our module that is going to read the encoder values of the robot,
//calculate the velocity of each joint, and then turn on or off the
//event-stream
class saccadicSuppression : public yarp::os::RFModule
{

private:

    //parameters
    double vel_threshold;
    double update_period;

    //motor state readers
    yarp::os::BufferedPort<yarp::sig::Vector> torso_reader;
    yarp::os::BufferedPort<yarp::sig::Vector> head_reader;

    //velocity estimators
    iCub::ctrl::AWLinEstimator *torso_vel;
    iCub::ctrl::AWLinEstimator *head_vel;

    //private functions
    bool checkEyeMotion(); //check if the eyes are moving (head or torso)
    bool openJointReaders(std::string module_name); //open the ports for encs.

    //switchable input/output port
    eventGatePort event_gate;

public:

    saccadicSuppression();
    ~saccadicSuppression();
    virtual bool configure(yarp::os::ResourceFinder &rf);

    virtual bool interruptModule();
    virtual bool close();
    virtual double getPeriod();
    virtual bool updateModule();

};
