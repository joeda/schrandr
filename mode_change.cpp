#include <set>

#include "mode_change.h"

namespace schrandr {

    ModeChange::ModeChange(
        const Mode &current,
        const Mode &target,
        const std::map<Edid, std::vector<xcb_randr_crtc_t> > &allCrtcsByEdid,
        const std::map<Edid, xcb_randr_output_t> &rawOutputsByEdid)
    {
        std::cout << "------SET MODE------\n\n" << std::endl;
        std::vector<std::vector<Edid > > edidsCurrent, edidsTarget;
        for (const auto &crtc : current.get_screens().front().get_crtcs()) {
            std::vector<Edid> edidsInCrtc;
            for (const auto &op : crtc.outputs) {
                edidsInCrtc.push_back(op.edid);
            }
            edidsCurrent.push_back(edidsInCrtc);
        }
        for (const auto &crtc : target.get_screens().front().get_crtcs()) {
            std::vector<Edid> edidsInCrtc;
            for (const auto &op : crtc.outputs) {
                edidsInCrtc.push_back(op.edid);
            }
            edidsTarget.push_back(edidsInCrtc);
        }
        auto edidDiff = util::compareVectors(edidsCurrent, edidsTarget);
        auto edidsKept = util::intersect(edidsCurrent, edidsTarget);
        std::cout << "EDIDs added: " << std::endl;
        for (const auto &edidVec : edidDiff.first) {
            std::cout << "CRTC" << std::endl;
            for (const auto &edid : edidVec) {
                std::cout << "\t" << edid.to_string() << std::endl;
            }
        }
        std::cout << "EDIDs removed: " << std::endl;
        for (const auto &edidVec : edidDiff.second) {
            std::cout << "CRTC" << std::endl;
            for (const auto &edid : edidVec) {
                std::cout << "\t" << edid.to_string() << std::endl;
            }
        }
        std::cout << "EDIDs kept: " << std::endl;
        for (const auto &edidVec : edidsKept) {
            std::cout << "CRTC" << std::endl;
            for (const auto &edid : edidVec) {
                std::cout << "\t" << edid.to_string() << std::endl;
            }
        }
        std::cout << "Debug A3" << std::endl;
        auto changeMap = makeCrtcDiff_(current,
                                       target,
                                       allCrtcsByEdid);
        std::cout << "ChangeMap:" << std::endl;
        for (const auto &pair : changeMap) {
            std::cout << "Info for CRTC " << pair.first << ":\n"
                      << "Pre:\n" << pair.second.first << "\nPost:\n"
                      << pair.second.second << std::endl;
        }
        std::cout << "Debug A4" << std::endl;
        for (auto &pair : changeMap) {
            bool handled = false;
            if (pair.second.second.isEmpty()) {
                crtcsToDisable_.push_back(pair.first);
                continue;
            }
            pair.second.second.crtc = pair.first;
            for (auto &op : pair.second.second.outputs) {
                op.output = rawOutputsByEdid.at(op.edid);
            }
            crtcChanges_.push_back(pair.second.second);
        }
        if (current.get_screens().front() != target.get_screens().front()) {
            screenChanges_.push_back(target.get_screens().front());
        }
    }
    
    std::map<xcb_randr_crtc_t, std::pair<CRTC, CRTC> > ModeChange::makeCrtcDiff_(
        const Mode &current,
        const Mode &target,
        const std::map<Edid, std::vector<xcb_randr_crtc_t> > &allCrtcsByEdid)
    {
        std::cout << "Debug A5" << std::endl;
        std::map<xcb_randr_crtc_t, std::pair<CRTC, CRTC> > result;
        std::set<xcb_randr_crtc_t> allCrtcsRaw;
        for (const auto &pair : allCrtcsByEdid) {
            for (const auto &crtc :  pair.second) {
                allCrtcsRaw.insert(crtc);
            }
        }
        std::vector<xcb_randr_crtc_t> crtcsTaken;
        std::vector<CRTC> curCrtcs, targetCrtcs, handledCur;
        std::cout << "Debug A6" << std::endl;
        for (const auto &screen : current.get_screens()) {
            std::vector<CRTC> crtcs = screen.get_crtcs();
            curCrtcs.insert(curCrtcs.end(), crtcs.begin(), crtcs.end());
        }
        std::cout << "Debug A6.5" << std::endl;
        for (const auto &screen : target.get_screens()) {
            std::vector<CRTC> crtcs = screen.get_crtcs();
            targetCrtcs.insert(targetCrtcs.end(), crtcs.begin(), crtcs.end());
        }
        std::cout << "Debug A7" << std::endl;
        for (auto it = targetCrtcs.begin(); it != targetCrtcs.end(); ) {
            auto possibleCrtcs = getPossibleRawCrtcs_(*it, allCrtcsByEdid);
            for (const auto &taken : crtcsTaken) {
                possibleCrtcs.erase(std::remove(possibleCrtcs.begin(),
                                                possibleCrtcs.end(),
                                                taken),
                                    possibleCrtcs.end());
            }
            bool matched = false;
            for (const auto &possibleCrtc : possibleCrtcs) {
                if (matched) {
                    break;
                }
                for (const auto &curCrtc : curCrtcs) {
                    if ((curCrtc.crtc == possibleCrtc) && 
                        curCrtc.hasSameEdids(*it)) {
                            result[possibleCrtc] = std::make_pair(curCrtc, *it);
                            matched = true;
                            crtcsTaken.push_back(possibleCrtc);
                            break;
                        }
                }
            }
            if (matched) {
                it = targetCrtcs.erase(it);
            } else {
                ++it;
            }
        }
        for (auto it = targetCrtcs.begin(); it != targetCrtcs.end(); ) {
            auto possibleCrtcs = getPossibleRawCrtcs_(*it, allCrtcsByEdid);
            for (const auto &taken : crtcsTaken) {
                possibleCrtcs.erase(std::remove(possibleCrtcs.begin(),
                                                possibleCrtcs.end(),
                                                taken),
                                    possibleCrtcs.end());
            }
            bool matched = false;
            for (const auto &possibleCrtc : possibleCrtcs) {
                if (matched) {
                    break;
                }
                for (const auto &curCrtc : curCrtcs) {
                    if (curCrtc.crtc == possibleCrtc) {
                            result[possibleCrtc] = std::make_pair(curCrtc, *it);
                            matched = true;
                            crtcsTaken.push_back(possibleCrtc);
                            break;
                        }
                }
            }
            if (matched) {
                it = targetCrtcs.erase(it);
            } else {
                ++it;
            }
        }
        for (auto it = targetCrtcs.begin(); it != targetCrtcs.end(); ) {
            auto possibleCrtcs = getPossibleRawCrtcs_(*it, allCrtcsByEdid);
            for (const auto &taken : crtcsTaken) {
                possibleCrtcs.erase(std::remove(possibleCrtcs.begin(),
                                                possibleCrtcs.end(),
                                                taken),
                                    possibleCrtcs.end());
            }
            bool matched = false;
            if (!possibleCrtcs.empty()) {
                CRTC empty;
                result[possibleCrtcs.front()] = std::make_pair(empty, *it);
                matched = true;
                crtcsTaken.push_back(possibleCrtcs.front());
                it = targetCrtcs.erase(it);
            } else {
                ++it;
                std::cout << "Warning: Could not match CRTC!" << std::endl;
            }
        }
        std::cout << "Debug A8" << std::endl;
        for (const auto &crtc : allCrtcsRaw) {
            if (result.find(crtc) != result.end()) {
                continue;
            }
            for (const auto &curCrtc : curCrtcs) {
                if (curCrtc.crtc == crtc) {
                    CRTC empty;
                    result[crtc] = std::make_pair(curCrtc, empty);
                }
            }
        }
        std::cout << "Debug A9" << std::endl;
        return result;
    }
    
    std::vector<xcb_randr_crtc_t> ModeChange::getPossibleRawCrtcs_(
        const CRTC &crtc,
        const std::map<Edid, std::vector<xcb_randr_crtc_t> > &allCrtcsByEdid)
    {
        std::set<xcb_randr_crtc_t> allCrtcsRaw;
        for (const auto &pair : allCrtcsByEdid) {
            for (const auto &crtc :  pair.second) {
                allCrtcsRaw.insert(crtc);
            }
        }
        std::vector<xcb_randr_crtc_t> possible(allCrtcsRaw.begin(), allCrtcsRaw.end());
        for (const auto &op : crtc.outputs) {
            possible = util::intersect(allCrtcsByEdid.at(op.edid), possible);
        }
        
        return possible;
    }
    
    std::vector<xcb_randr_crtc_t> ModeChange::getCrtcsToDisable() const {
        return crtcsToDisable_;
    }
    
    std::vector<CRTC> ModeChange::getCrtcChanges() const {
        return crtcChanges_;
    }
    
    std::vector<Screen> ModeChange::getScreenChanges() const {
        return screenChanges_;
    }
}
