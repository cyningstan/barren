//
// Barren Planet
// A turn-based strategy game.
//
// Copyright (C) Damian Gareth Walker 2020. Released under the GNU GPL.
// Source file for the manual cover and title screen image.
//

//----------------------------------------------------------------------
// Configuration
//

// Includes
#include "colors.inc"
#include "textures.inc"
#include "ground.inc"
#include "sky.inc"
#include "mining-base.inc"
#include "hoverbug.inc"
#include "ground-rover.inc"
#include "gun-platform.inc"

// Camera
camera {
  location <0, 20, -150>
  look_at <0, 20, 0>
  //right image_width / image_height * x
  right x
  up image_height / image_width * y
}

// Light Source
light_source {
  <1000, 500, -1000>
  color White
}

//----------------------------------------------------------------------
// The Environment
//

object {
  Ground
}
sky_sphere {
  Sky
}
Cloud

//----------------------------------------------------------------------
// The Units
//

// Mining Base
object {
  MiningBase
  rotate <0, 30, 0>
  translate <30, 0, 30>
}

// Hoverbugs
object {
  Hoverbug
  rotate <0, -60, 0>
  translate <50, 0, -30>
}
object {
  Hoverbug
  rotate <0, -60, 0>
  translate <40, 0, -40>
}
object {
  Hoverbug
  rotate <0, -60, 0>
  translate <30, 0, -50>
}

// Ground Rovers
object {
  GroundRover
  rotate <0, 45, 0>
  translate <-20, 0, -50>
}
object {
  GroundRover
  rotate <0, 45, 0>
  translate <-40, 0, -30>
}
object {
  GunPlatform
  rotate <0, -30, 0>
  translate <-40, 0, 50>
}