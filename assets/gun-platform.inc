//
// Barren Planet
// A turn-based strategy game.
//
// Copyright (C) Damian Gareth Walker 2022.
// POV-Ray source include for the Gun Platform
//

// Declare the Gun Platform
#declare GunPlatform =
union {

  // The base
  cone {
    <0,0,0>, 15
    <0,5,0>, 7.5
  }
  union {
    difference {
      cylinder {
        <0,0,0>
        <0,10,0>,
        5
      }
      cylinder {
        <0,2.5,0>
        <0,7.5,0>
        6
      }
    }
    cylinder {
      <0,0,0>
      <0,10,0>,
      3.75
    }
    rotate <0,0,90>
    translate <5,5,0>
  }

  // The superstructure
  box {
    <-2.5,5,-2.5>
    <2.5,40,2.5>
    rotate <30,0,0>
  }

  // The gun and gun barrel
  union {
    cylinder {
      <0,0,0>
      <0,7.5,0>
      10
    }
    cone {
      <0, 7.5, 0> 10
      <0, 10, 0> 5
    }
    cylinder {
      <0,5,0>
      <0,5,-40>
      2
    }
    difference {
      cylinder {
        <0,5,0>
        <0,5,-80>
        1
      }
      cylinder {
        <0,5,-70>
	<0,5,-81>
	0.5
      }
      pigment {
        color rgb <.333, .333, .333>
      }
    }
    translate <0,32.5,17.5>
  }

  // The overall texture
  pigment {
    color White
  }
}