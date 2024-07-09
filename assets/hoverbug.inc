//
// Barren Planet
// A turn-based strategy game.
//
// Copyright (C) Damian Gareth Walker 2022.
// POV-Ray source include for the Hoverbug.
//

// Declare the Hoverbug
#declare Hoverbug =
union {
  cone {
    <0,1,0>, 6,
    <0,2,0>, 4.5
    pigment {
      color White
    }
  }
  difference {
    sphere {
      <0,2,0>,
      1
      scale <2,1,4.5>
    }
    box {
      <-2,2,-4.5>,
      <2,3,0>
    }
    pigment {
      color White
    }
  }
  cylinder {
    <0,1,3>,
    <0,4,3>,
    1
    pigment {
      color White
    }
  }
  cylinder {
    <0,3.5,3>,
    <0,4,0>,
    0.25
    pigment {
      color rgb <.333, 0.333, 0.333>
    }
  }
  sphere {
    <0,2,0>,
    0.875
    scale <2,1,4.5>
    pigment {
      color rgbt <.333, .333, .333, .25>
    }
  }
}

