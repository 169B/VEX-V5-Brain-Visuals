/**
 * @file routines.cpp
 * @brief Example Autonomous Routines for Demonstration
 */

#include "auton/selector.hpp"
#include "api.h"
#include <iostream>

// Example autonomous routine: Left side - 4 rings
void auto_left_4ring() {
    std::cout << "Running Left 4-Ring Auto..." << std::endl;
    
    // Example motor commands (would need real motors in actual code)
    // For demonstration, just print the actions
    
    std::cout << "  Moving forward..." << std::endl;
    pros::delay(500);
    
    std::cout << "  Turning left..." << std::endl;
    pros::delay(300);
    
    std::cout << "  Collecting ring 1..." << std::endl;
    pros::delay(400);
    
    std::cout << "  Moving to ring 2..." << std::endl;
    pros::delay(500);
    
    std::cout << "  Collecting ring 2..." << std::endl;
    pros::delay(400);
    
    std::cout << "  Moving to ring 3..." << std::endl;
    pros::delay(500);
    
    std::cout << "  Collecting ring 3..." << std::endl;
    pros::delay(400);
    
    std::cout << "  Moving to ring 4..." << std::endl;
    pros::delay(500);
    
    std::cout << "  Collecting ring 4..." << std::endl;
    pros::delay(400);
    
    std::cout << "  Returning to start..." << std::endl;
    pros::delay(600);
    
    std::cout << "Left 4-Ring Auto complete!" << std::endl;
}

// Example autonomous routine: Right side - 4 rings
void auto_right_4ring() {
    std::cout << "Running Right 4-Ring Auto..." << std::endl;
    
    std::cout << "  Moving forward..." << std::endl;
    pros::delay(500);
    
    std::cout << "  Turning right..." << std::endl;
    pros::delay(300);
    
    std::cout << "  Collecting rings..." << std::endl;
    pros::delay(2000);
    
    std::cout << "  Returning to start..." << std::endl;
    pros::delay(600);
    
    std::cout << "Right 4-Ring Auto complete!" << std::endl;
}

// Example autonomous routine: Center - AWP
void auto_center_awp() {
    std::cout << "Running Center AWP Auto..." << std::endl;
    
    std::cout << "  Moving to alliance stake..." << std::endl;
    pros::delay(700);
    
    std::cout << "  Scoring on alliance stake..." << std::endl;
    pros::delay(500);
    
    std::cout << "  Moving to ladder..." << std::endl;
    pros::delay(800);
    
    std::cout << "  Climbing ladder..." << std::endl;
    pros::delay(1000);
    
    std::cout << "Center AWP Auto complete!" << std::endl;
}

// Example autonomous routine: Safe - just touch ladder
void auto_safe() {
    std::cout << "Running Safe Auto..." << std::endl;
    
    std::cout << "  Moving forward slowly..." << std::endl;
    pros::delay(1500);
    
    std::cout << "  Touching ladder..." << std::endl;
    pros::delay(500);
    
    std::cout << "Safe Auto complete!" << std::endl;
}

// Example skills routine: Full field
void skills_full() {
    std::cout << "Running Full Skills Auto..." << std::endl;
    
    std::cout << "  Phase 1: Clearing left side..." << std::endl;
    pros::delay(5000);
    
    std::cout << "  Phase 2: Moving to center..." << std::endl;
    pros::delay(3000);
    
    std::cout << "  Phase 3: Clearing right side..." << std::endl;
    pros::delay(5000);
    
    std::cout << "  Phase 4: Scoring all rings..." << std::endl;
    pros::delay(3000);
    
    std::cout << "  Phase 5: Climbing..." << std::endl;
    pros::delay(2000);
    
    std::cout << "Full Skills Auto complete!" << std::endl;
}

// Example skills routine: Safe skills
void skills_safe() {
    std::cout << "Running Safe Skills Auto..." << std::endl;
    
    std::cout << "  Scoring preload..." << std::endl;
    pros::delay(1000);
    
    std::cout << "  Collecting nearby rings..." << std::endl;
    pros::delay(3000);
    
    std::cout << "  Climbing ladder..." << std::endl;
    pros::delay(2000);
    
    std::cout << "Safe Skills Auto complete!" << std::endl;
}

// Register all autonomous routines
// These use the REGISTER macros to auto-register before main()
REGISTER_MATCH_AUTO("Left 4-Ring", "Score 4 rings on left side, return to start", auto_left_4ring);
REGISTER_MATCH_AUTO("Right 4-Ring", "Score 4 rings on right side, return to start", auto_right_4ring);
REGISTER_MATCH_AUTO("Center AWP", "Score alliance stake and climb for AWP", auto_center_awp);
REGISTER_MATCH_AUTO("Safe", "Just touch the ladder - guaranteed points", auto_safe);

REGISTER_SKILLS_AUTO("Full Field", "Complete 60-second skills run - max points", skills_full);
REGISTER_SKILLS_AUTO("Safe Skills", "Conservative skills run - guaranteed climb", skills_safe);
