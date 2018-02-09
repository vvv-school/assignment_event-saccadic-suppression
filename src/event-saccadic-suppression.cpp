#include "event-saccadic-suppression.h"
#include <math.h>

/******************************************************************************/
//main
/******************************************************************************/

int main(int argc, char * argv[])
{
    /* initialize yarp network */
    yarp::os::Network yarp;
    if(!yarp.checkNetwork()) {
        yError() << "Could not connect to yarp";
        return false;
    }

    /* prepare and configure the resource finder */
    yarp::os::ResourceFinder rf;
    rf.setDefaultConfigFile("saccadic-suppression.ini");
    rf.setDefaultContext("eventdriven");
    rf.configure(argc, argv);

    /* instantiate the module */
    saccadicSuppression mymodule;
    return mymodule.runModule(rf);
}

/******************************************************************************/
//eventGatePort
/******************************************************************************/
void eventGatePort::activate()
{
    //active allows the gate to be open
    active = true;
}

void eventGatePort::disactivate()
{
    //disactivate blocks the event-stream
    active = false;
}

bool eventGatePort::open(const yarp::os::ConstString &name)
{
    //set up the eventGatePort and open input and output ports
    setStrict();
    useCallback();
    return output_port.open(name + "/vBottle:o") &&
            yarp::os::BufferedPort<vBottle>::open(name + "/vBottle:i");
}

void eventGatePort::interrupt()
{
    //interrupt evreything
    disactivate();
    yarp::os::BufferedPort<vBottle>::interrupt();
    output_port.interrupt();
}

void eventGatePort::close()
{
    //close everything
    yarp::os::BufferedPort<vBottle>::close();
    output_port.close();
}

eventGatePort::~eventGatePort()
{
    //destruct everything
    yarp::os::BufferedPort<vBottle>::close();
    output_port.close();
}

void eventGatePort::onRead(vBottle &input)
{
    //if active pass through the bottle from input to output
    if(active) {

        //copy the timestamp to the output port
        //copy the data to the output port

        // FILL IN CODE HERE

    }

    //else do nothing;

}


/******************************************************************************/
//saccadicSuppression
/******************************************************************************/

saccadicSuppression::saccadicSuppression()
{
    //creat the linear estimators
    vel_threshold = 0.1;
    torso_vel = new iCub::ctrl::AWLinEstimator(5, 0.5);
    head_vel = new iCub::ctrl::AWLinEstimator(5, 0.5);
}

saccadicSuppression::~saccadicSuppression()
{
    //interrupt and close everything
    //delete the linear estimators
    interruptModule();
    close();
    delete torso_vel;
    delete head_vel;
}

bool saccadicSuppression::configure(yarp::os::ResourceFinder &rf)
{

    //set the parameters using the RFModule configure magic
    vel_threshold =
            rf.check("threshold", yarp::os::Value(0.1)).asDouble();
    update_period =
            rf.check("update", yarp::os::Value(0.001)).asDouble();
    std::string port_name =
            rf.check("name", yarp::os::Value("/vSacSup")).asString();

    //return true only if all ports are open correctly
    return openJointReaders(port_name) && event_gate.open(port_name);
}


double saccadicSuppression::getPeriod()
{
    //this will control the rate at which we are checking the encoders
    return update_period;
}

bool saccadicSuppression::updateModule()
{
    //check the eye motion and activate/disactivate the event-stream gate

    // FILL IN CODE HERE

    return !isStopping();
}


bool saccadicSuppression::checkEyeMotion()
{

    bool torso_changed = false;
    bool head_changed = false;
    //read the torso values and feed them to the linear estimator
    //if the estimate from the linear estimator is above our threshold
    //return false;
    //do the same for the head values
    //otherwise return true

    //FILL IN CODE HERE

    return head_changed || torso_changed;
}

bool saccadicSuppression::openJointReaders(std::string module_name)
{

    //open ports for reading the torso and head encoder values
    //these ports should be names /icub/torso/state:o and
    // /icub/head/state:o
    //return false if all ports aren't open

    //FILL IN CODE HERE

    return true;

}


bool saccadicSuppression::interruptModule()
{
    //interrupt everything
    event_gate.interrupt();
    torso_reader.interrupt();
    head_reader.interrupt();
    return yarp::os::RFModule::interruptModule();
}

bool saccadicSuppression::close()
{
    //close everything
    event_gate.close();
    torso_reader.close();
    head_reader.close();
    return yarp::os::RFModule::close();
}






