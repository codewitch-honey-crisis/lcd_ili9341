# ILI9341 Display Driver and demo for GFX

This code is a playground for driver development, but right now it supports the ILI9341 as write only, but it's very snappy in terms of performance, implementing every performance capability of GFX

GFX meanwhile, is the drawing and graphics library that can interface with this or other drivers. They are independent of each other, although for a driver to work with GFX it must expose GFX bindings and thus have a dependency on GFX

