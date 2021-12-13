# Notes on how to set up a Patcher and Formula Controller mapping to allow bitmasking in the output CC value

 - The new trigger_on functionality for envelopes uses bitmasking to cram extra options into the same CC value.
 - ie, values 0-19 specify the trigger number to respond to.  setting the 6th bit aka 32 indicates to enable loop.  setting 7th bit aka 64 indicates to enable invert.  These options can be combined and the trigger number to respond to is also still honoured.
 - Effectively this means 0-19 are trigger number, 20 is 'no trigger', 32-51 are trigger number+loop, 52 is 'no trigger+loop', 64-83 are trigger number+invert, 84 is 'no trigger+invert', 96-115 is trigger number+loop+invert, 116 is 'no trigger+loop+invert'
 - To set this up in FL Studio Patcher/Control Surface/Formula Controller:

 * Add a Digit selector for selecting the trigger number - set range of 0-20
 * Add a Checkbox for each option, ie a Loop and an Invert checkbox
 * Add a Formula Controller
 * Connect the Digit output with Formula Controller input a
 * Connect the Loop output with Formula Controller input b
 * Connect the Invert output with the Formula Controller input c
 * Use formula `((a*20) + (b*32) + (c*64) ) /127` in the Formula Controller
 * Select the MIDI Out used and configure the control to the appropriate envelope channel and CC
 * Set it to Range 0-127
 * Take the output of Formula Controller into the parameter input for the appropriate control
 * Should be all you need!

