//
// Barren Planet
// A turn-based strategy game.
//
// Copyright (C) Damian Gareth Walker 2022.
// POV-Ray source include for the Mining Base.
//

// Declare the mining base object
#declare MiningBase =
union {

  // The lower part
  difference {
    union {
      cone {
	<0,0,0>, 20
	<0,30,0>, 15
      }
      box {
	<-7.5,0,-20>
	<7.5,20,0>
      }
      pigment {
	color White
      }
    }
    box {
      <-6.25,0,-18.75>
      <6.25,18.75,-21.25>
      pigment {
	color White
      }
    }
  }

  // The little comms light things
  #declare Angle = 0;
  #while (Angle <= 180)
    union {
      cylinder {
	<20,0,0>,
	<20,5,0>,
	2.5
	pigment {
	  color White
	}
      }
      cylinder {
	<20,0,0>
	<20,13.75,0>,
	.625
	pigment {
	  color rgb <.333, .333, .333>
	}
      }
      sphere {
	<20,15,0>,
	1.25
	pigment {
	  color rgbt <1, .333, .333, .25>
	}
      }
      light_source {
	<20,15,0>
	color White
      }
      rotate <0, Angle, 0>
    }
    #declare Angle = Angle + 180;
  #end

  // The rolling door
  #declare Base = 0;
  #while (Base < 18.75)
    // A single slat
    prism {
      -6.25,
      6.25,
      4,
      <0,-18.75>,
      <0,-20>,
      <1.25,-18.75>,
      <0,-18.75>
      pigment {
	color rgb <.333,.333,.333>
      }
      rotate <0, 0, 90>
      translate <0, Base, 0>
    }
    #declare Base = Base + 1.25;
  #end

  // The upper part
  union {
    sphere {
      <0,30,0>
      15
      pigment {
	color rgbt <1, 1, 1, .5>
      }
    }
    torus {
      15,
      0.3125
      pigment {
        color rgb <0.333, 0.333, 0.333>
      }
      translate <0, 30, 0>
    }
  }

}

