//
// Barren Planet
// A turn-based strategy game.
//
// Copyright (C) Damian Gareth Walker 2020. Released under the GNU GPL.
// POV-Ray Source file for the ground.
//

// Declare normal ground texture
#declare GroundTexture =
  texture {
    pigment {
      color rgb <1, .33, .33>
    }
    normal {
      bumps 0.75
    }
  }

// Declare the flat ground
#declare Foreground =
plane {
  <0, 1, 0>, 0
  translate <-.5, -.5, -.5>
}

#declare Hill =
height_field {
  png "landscape.png"
  smooth
  translate <-.5, -.5, .1>
  scale <1e4, 250, 1e4>
}

#declare Ground =
union {
  object {
    Hill
  }
  object {Foreground}
  texture {GroundTexture}
}