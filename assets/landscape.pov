// Landscape functions

#include "functions.inc"

#declare IsTest = 0;
#declare IsWater = 0;

#global_settings {
  #if(!IsTest) hf_gray_16 on #end
  number_of_waves 35
}

#declare aspect = image_width/image_height;
#declare aperSize = 3000;

#declare l0Pat = function { f_ridged_mf(x,y,z,0.01,3.58,19,-.0,3.1,3) }

#declare l1Pat = function { pattern {
  crackle
  metric 1.4
  form <1.0,-0.2, 0.3>
  scale 1400
} }

#declare l2Pat = function { pattern {
  granite
  scale 4900
} }

#declare w1Pat = function { pattern {
  waves
  triangle_wave
  frequency 12
  turbulence 0.21
  scale 3000
} }

#declare w2Pat = function { pattern {
  ripples
  triangle_wave
  frequency 43
  turbulence 0.21
  scale 5000
} }

#declare l3Pat = function { pattern {
  leopard
  sine_wave
  turbulence 1.21
  octaves 8
  omega 0.73
  scale 800
} }

#if (IsWater)
  #declare lkpat = function { pow(w1Pat(x,y,z),0.5)*.70 + 
pow(w2Pat(x,0,z),2.0)*0.30 }
#else
  #declare lkpat = function { (1-pow(l1Pat(x,y,z),1.2))*.88 + 
pow(l2Pat(x,0,z),1.2)*0.15+pow(l3Pat(x,0,z),2)*0.20 }
#end

plane { y, 0
  texture {
    pigment {
      function { lkpat(x,y,z) }
      color_map {
        [0.00 rgb (IsTest ? <0,1,1> : <0,0,0>) ]
        [1.00 rgb (IsTest ? <1,0,0> : <1,1,1>) ]
      }
    }
    finish { ambient 1 }
  }
}

camera {
  orthographic
  location < 0.0, 2.0, 0.0> look_at < 0.0, 0.0, 0.0>
  right x*aperSize*aspect
  up z*aperSize
}