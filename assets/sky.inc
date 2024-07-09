//
// Barren Planet
// A turn-based strategy game.
//
// Copyright (C) Damian Gareth Walker 2020. Released under the GNU GPL.
// POV-Ray Source file for the ground.
//

#declare Sky =
sky_sphere {
  pigment {
    gradient y
    color_map {
      [ 0.5  color rgb <0.33, 1, 1> ]
      [ 1.0  color rgb <1, 0.33, 0.33> ]
    }
    scale 2
    translate -1
  }
}

// The Cloud Layer
#declare Cloud =
plane {
  <0, 1, 0>, 1000
  pigment {
    bozo
    turbulence 0.65
    octaves 6
    omega 0.7
    lambda 2
    color_map {
      [0.0 0.1 color rgb <0.85, 0.85, 0.85>
	color rgb <0.75, 0.75, 0.75>]
      [0.1 0.5 color rgb <0.75, 0.75, 0.75>
	color rgbt <1, 1, 1, 1>]
      [0.5 1.0 color rgbt <1, 1, 1, 1>
	color rgbt <1, 1, 1, 1>]
    }
    scale <2000, 5000, 2000>
  }
}
