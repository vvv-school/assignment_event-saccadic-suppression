/*
 * Copyright (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Ugo Pattacini <ugo.pattacini@iit.it>
 * CopyPolicy: Released under the terms of the GNU GPL v3.0.
*/
#include <string>
#include <deque>
#include <fstream>
#include <unistd.h>

#include <yarp/robottestingframework/TestCase.h>
#include <robottestingframework/dll/Plugin.h>
#include <robottestingframework/TestAssert.h>


#include <yarp/os/all.h>
#include <iCub/eventdriven/all.h>

using namespace std;
using namespace robottestingframework;

class bottleNumberChecker : public yarp::os::BufferedPort< ev::vBottle >
{

private:



    std::deque<int> gt;
    std::deque<int> td;

    void onRead(ev::vBottle &input) {

        yarp::os::Stamp yarpstamp;
        getEnvelope(yarpstamp);
        td.push_back(yarpstamp.getCount());

    }

public:

    double fnratio;
    int tn;

    bottleNumberChecker()
    {
        this->useCallback();
        this->setStrict();
    }

    void performComparison() {

        double false_negatives = 0;
        double max_fp = 1240;
        for(size_t i = 1; i < td.size(); i++) {
            if(td[i] > 1950 && td[i] < 2150)
                false_negatives++;
            else if(td[i] > 2625 && td[i] < 2910)
                false_negatives++;
            else if(td[i] > 3325 && td[i] < 3530)
                false_negatives++;
            else if(td[i] > 4245 && td[i] < 4505)
                false_negatives++;
            else if(td[i] > 4870 && td[i] < 5160)
                false_negatives++;
            else if(td[i] < 0 || td[i-1] >= td[i])
                false_negatives++;
        }
        fnratio = false_negatives / max_fp;
        tn = td.size() - false_negatives;

    }





//        //this code expects td and gt to be monotonically increasing

//        std::cout << "Ground Truth: " << gt.size() << " " << gt.front() << "->" << gt.back() << std::endl;
//        if(td.size())
//            std::cout << "TD: " << td.size() << " " << td.front() << "->" << td.back() << std::endl;
//        else
//            std::cout << "TD: 0";

//        unsigned int i = 0;
//        unsigned int j = 0;
//        unsigned int false_negative = 0;
//        unsigned int false_positive = 0;
//        //int total = std::max(gt.back(), td.back());

//        while(i < gt.size() && j < td.size()) {
//            if(gt[i] < td[j]) {
//                i++;
//                false_positive++;
//            } else if(td[j] < gt[i]) {
//                j++;
//                false_negative++;
//            } else {
//                i++;
//                j++;
//            }

//        }

//        //there might be more data in td
//        false_negative += td.size() - j;
//        false_positive += gt.size() - i;

//        //final result
//        unsigned int total_error = false_negative + false_positive;
//        unsigned int total = gt.back() - gt.size();
//        total = std::max(total, total_error);
//        return (100 * (total - total_error)) / total;

//    }

    bool loadGroundTruth(std::string filename)
    {
        //char buff[1024];
        //getcwd(buff, 1024);
        //std::cout << buff << std::endl;
        int bn;
        std::ifstream gtreader;
        gtreader.open(filename.c_str());
        if(!gtreader.is_open())
            return false;

        while(gtreader >> bn)
            gt.push_back(bn);

        gtreader.close();

        std::cout << "==================" << std::endl;
        std::cout << "GT: " << gt.front() << " -> " << gt.back() << " | " <<
                     gt.size() << "/" << gt.back() << std::endl;
        std::cout << "==================" << std::endl;

        if(gt.size() > 0)
            return true;
        else
            return false;
    }

    bool saveResult(std::string filename)
    {
        //char buff[1024];
        //getcwd(buff, 1024);
        //std::cout << buff << std::endl;

        std::ofstream tdsaver;
        tdsaver.open(filename.c_str());
        if(!tdsaver.is_open())
            return false;

        for(size_t i = 0; i < td.size(); i++)
            tdsaver << td[i] << std::endl;

        tdsaver.close();

    }


};


/**********************************************************************/
class TestAssignmentEventSaccadicSuppression : public yarp::robottestingframework::TestCase
{

private:

    bottleNumberChecker checker;
    yarp::os::RpcClient playercontroller;


public:
    /******************************************************************/
    TestAssignmentEventSaccadicSuppression() :
        yarp::robottestingframework::TestCase("TestAssignmentEventSaccadicSuppression")
    {
    }

    /******************************************************************/
    virtual ~TestAssignmentEventSaccadicSuppression()
    {
    }

    /******************************************************************/
    virtual bool setup(yarp::os::Property& property)
    {

        //we need to load the data file into yarpdataplayer
        std::string cntlportname = "/playercontroller/rpc";

        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(playercontroller.open(cntlportname),
                                  "Could not open RPC to yarpdataplayer");

        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(yarp::os::Network::connect(cntlportname, "/yarpdataplayer/rpc:i"),
                                  "Could not connect RPC to yarpdataplayer");

        //we need to check the output of yarpdataplayer is open and input of spiking model
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(yarp::os::Network::connect("/zynqGrabber/vBottle:o", "/vSacSup/vBottle:i", "udp"),
                                  "Could not connect yarpdataplayer to assignment module (events)");

        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(yarp::os::Network::connect("/icub/torso/state:o", "/vSacSup/torso/state:i", "udp"),
                                  "Could not connect yarpdataplayer to assignment module (torso)");

        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(yarp::os::Network::connect("/icub/head/state:o", "/vSacSup/head/state:i", "udp"),
                                  "Could not connect yarpdataplayer to assignment module (head)");

        //check we can open our spike checking consumer
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(checker.open("/checker/vBottle:i"),
                                  "Could not open checker");

        //the output of spiking model
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(yarp::os::Network::connect("/vSacSup/vBottle:o", "/checker/vBottle:i", "udp"),
                                  "Could not connect assignment to checker");

        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Ports successfully open and connected");

//        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(checker.loadGroundTruth("../bottle_numbers.csv"),
//                                  "Could not load ground truth");

        return true;
    }

    /******************************************************************/
    virtual void tearDown()
    {
        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Closing Clients");
        playercontroller.close();
        checker.close();
    }

    /******************************************************************/
    virtual void run()
    {

        //play the dataset

        yarp::os::Bottle cmd, reply;
        cmd.addString("play");
        playercontroller.write(cmd, reply);
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(reply.get(0).asString() == "ok", "Did not successfully play the dataset");

        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Playing dataset - please wait till it finishes automatically");
        yarp::os::Time::delay(30);

        cmd.clear();
        cmd.addString("stop");
        playercontroller.write(cmd, reply);
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(reply.get(0).asString() == "ok", "Did not successfully stop the dataset");
        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Stopping dataset - wait for score calculation");

        checker.performComparison();

        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("Percentage of saccade not blocked: %f", checker.fnratio));
        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("Total correctly not blocked: %d", checker.tn));

        int score = 0;
        //if a metric is too low give 0
        if(checker.fnratio > 0.8 ) {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Not enough blocked events(0)");
        } else if(checker.tn < 1000) {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Too many events blocked(0)");
        } else {
            if(checker.fnratio < 0.1) {
                ROBOTTESTINGFRAMEWORK_TEST_REPORT("Blocked Events: max points(3)");
                score += 3;
            } else if(checker.fnratio < 0.3) {
                ROBOTTESTINGFRAMEWORK_TEST_REPORT("Blocked Events: good(2)");
                score += 2;
            } else if(checker.fnratio < 0.5) {
                ROBOTTESTINGFRAMEWORK_TEST_REPORT("Blocked Events: passed(1)");
                score += 1;
            }

            if(checker.tn > 5500) {
                ROBOTTESTINGFRAMEWORK_TEST_REPORT("Non-blocked Events: max points(3)");
                score += 3;
            } else if(checker.tn > 4500) {
                ROBOTTESTINGFRAMEWORK_TEST_REPORT("Non-blocked Events: good(2)");
                score += 2;
            } else if(checker.tn > 3000) {
                ROBOTTESTINGFRAMEWORK_TEST_REPORT("Non-blocked Events: passed(1)");
                score += 1;
            }


        }


        //checker.saveResult("../bottle_numbers_td.csv");
        //ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("percentage = %d", score));
        //score = 0.5 + 3.0 * score / 100.0;
        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Maximum Score (6)");
        ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0, Asserter::format("Total score = %d", score));

//        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("Inliers = %d", inliers));
//        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("Outliers = %d", outliers));
//        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(inliers > 3000, "Inlier score too low (3000)");
//        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(outliers < 500, "Outlier score too high (500)");


    }
};

ROBOTTESTINGFRAMEWORK_PREPARE_PLUGIN(TestAssignmentEventSaccadicSuppression)
