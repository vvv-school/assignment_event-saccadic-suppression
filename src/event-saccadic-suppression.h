#include <yarp/os/all.h>
#include <iCub/ctrl/adaptWinPolyEstimator.h>
#include <string>
using namespace yarp::os;
using std::string;

/******************************************************************************/
//eventGatePort
/******************************************************************************/
//This class is an input port with a corresponding output port.
//Default operation is to send the input to the output
//By calling disactivate the input will be discarded instead
class eventGatePort : public yarp::os::BufferedPort<Bottle>
{
private:

    //gate state
    bool active;

    //output port (also this class has a port of it's own)
    BufferedPort<Bottle> output_port;

public:

    eventGatePort() : active(true) {}
    ~eventGatePort();

    void activate();
    void disactivate();

    virtual void interrupt();
    virtual void close();
    virtual void onRead(Bottle &input);
    virtual bool open(const string &name);

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
    BufferedPort<yarp::sig::Vector> torso_reader;
    BufferedPort<yarp::sig::Vector> head_reader;

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
