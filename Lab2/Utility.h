#pragma once

#define TAG_SPECIAL 1000000 // Ask for first task or tell there are no more tasks for this move
#define TAG_FINISH 2000000 // The game is over

bool approximately_equal(double a, double b);