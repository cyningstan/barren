//
// Barren Planet
// A turn-based strategy game.
//
// Copyright (C) Damian Gareth Walker 2022.
// POV-Ray source include for the Ground Rover.
//

// Wheel
#declare GroundRoverWheel =
union {
  union {
    cylinder {
      <-1,2,0>,
      <1,2,0>,
      1
    }
    cylinder {
      <-0.75,2,0>,
      <0.75,2,0>,
      1.5
    }
    pigment {
      color White
    }
  }
  union {
    torus {
      1.75
      0.25
      rotate <0,0,90>
      translate <-0.75,2,0>
    }
    torus {
      1.75
      0.25
      rotate <0,0,90>
      translate <0.75,2,0>
    }
    difference {
      cylinder {
	<-0.75,2,0>,
	<0.75,2,0>,
	2
      }
      cylinder {
	<-0.75,2,0>,
	<0.75,2,0>,
	1.5
      }
    }
    pigment {
      color rgb <0.333, 0.333, 0.333>
    }
  }
}

#declare GroundRover =
union {

  // Main body of the rover
  union {
    box {
      <-6,2,-9>,
      <6,4,9>
    }
    prism {
      conic_sweep
      linear_spline
      0.75, // height 1
      1, // height 2
      5, // the number of points making up the shape...
      <-6,-9>,<6,-9>,<6,9>,<-6,9>,<-6,-9>
      rotate <180, 0, 0>
      scale <1, 4, 1>
      translate <0, 8, 0>
    }
    prism {
      conic_sweep
      linear_spline
      0.75, // height 1
      1, // height 2
      5, // the number of points making up the shape...
      <-6,-9>,<6,-9>,<6,9>,<-6,9>,<-6,-9>
      scale <1, 8, 1>
      translate <0, -6, 0>
    }
    box {
      <-6,4,3>,
      <6,6,9>
    }
    prism {
      conic_sweep
      linear_spline
      0.75, // height 1
      1, // height 2
      5, // the number of points making up the shape...
      <-6,-3>,<6,-3>,<6,3>,<-6,3>,<-6,-3>
      rotate <180, 0, 0>
      scale <1, 4, 1>
      translate <0, 10, 6>
    }
    pigment {
      color White
    }
    translate <0,2,0>
  }

  // Wheels
  object {
    GroundRoverWheel
    translate <5,0,-6>
  }
  object {
    GroundRoverWheel
    translate <5,0,0>
  }
  object {
    GroundRoverWheel
    translate <5,0,6>
  }
  object {
    GroundRoverWheel
    translate <-5,0,-6>
  }
  object {
    GroundRoverWheel
    translate <-5,0,0>
  }
  object {
    GroundRoverWheel
    translate <-5,0,6>
  }

  // Gun Turret
  cylinder {
    <0, 7, -3>,
    <0, 9, -3>,
    2
    pigment {
      color White
    }
  }
  cylinder {
    <0, 8, -3>,
    <0, 8, -9>,
    0.25
    pigment {
      color rgb <.333, 0.333, 0.333>
    }
  }

  // Radar
  union {
    difference {
      sphere {
	<0, 0, 0>,
	3
      }
      sphere {
	<0, 0, 0>,
	2.75
      }
      box {
        <-3,0,-3>,
	<3,3,3>
      }
      pigment {
        color White
      }
      scale <0, .5, 0>
    }
    cylinder {
      <0,0,0>,
      <0,3,0>,
      0.25
      pigment {
	color rgb <.333, 0.333, 0.333>
      }
    }
    sphere {
      <0,3,0>,
      0.5
      pigment {
	color rgbt <1, .333, .333, .25>
      }
    }
    light_source {
      <0,3,0>
      color White
    }
    rotate <30,0,0>
    translate <0, 10, 6>
  }
}