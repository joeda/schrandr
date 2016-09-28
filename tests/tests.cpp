#include "../mode.h"
#include "../edid.h"

#include <gtest/gtest.h>

TEST(CRTC, empty) {
    schrandr::CRTC crtc1;
    schrandr::CRTC crtc2;
    ASSERT_TRUE(crtc1.hasSameEdids(crtc2));
    ASSERT_TRUE(crtc1.isEmpty());
}

TEST(Edid, empty) {
    schrandr::Edid edid;
    ASSERT_TRUE(edid.isDummy());
}

TEST(Edid, comparison) {
    schrandr::Edid edid1(
        "QqkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV");
    schrandr::Edid edid2(
        "QqkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV");
    schrandr::Edid edid3(
        "AKqkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV");
    
    ASSERT_TRUE(edid1 == edid2);
    ASSERT_FALSE(edid3 == edid1);
    ASSERT_FALSE(edid3 == edid2);
}

TEST(CRTC, comparison) {
    schrandr::CRTC crtc1;
    schrandr::CRTC crtc2;
    schrandr::CRTC crtc3;
    schrandr::Output o1 = {1,
        schrandr::Edid("QqkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV"),
        std::string("OUT1")
    };
    schrandr::Output o2 = {2,
        schrandr::Edid("QqkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV"),
        std::string("OUT2")
    };
    schrandr::Output o3 = {5,
        schrandr::Edid("AqkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV"),
        std::string("OUT8")
    };
    schrandr::Output o4 = {5,
        schrandr::Edid("AqkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV"),
        std::string("OUT9")
    };
    schrandr::Output o5 = {6,
        schrandr::Edid("XXkWlNstACxpSLQ4HBeLsQ9BBTf4DeyoiZGTLcJMcmOLFT5I9TMA5iW3BaVmgmBV"),
        std::string("OUT9")
    };
    
    crtc1.outputs.push_back(o1);
    crtc1.outputs.push_back(o3);
    crtc2.outputs.push_back(o4);
    crtc2.outputs.push_back(o2);
    crtc3.outputs.push_back(o1);
    crtc3.outputs.push_back(o5);
    
    ASSERT_TRUE(crtc1.hasSameEdids(crtc2));
    ASSERT_FALSE(crtc3.hasSameEdids(crtc1));
    ASSERT_FALSE(crtc3.hasSameEdids(crtc2));
}
 
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
