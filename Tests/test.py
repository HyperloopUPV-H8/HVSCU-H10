import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), "VirtualMCU", "src"))

from HVSCU import HVSCU
from runner import runner
from vmcu.assertions import *


@runner.test()
def HVSCU_Contactors_test():
    hvscu = HVSCU()

    
    
    
    check(
        HVSCU.is_state,
        args=(HVSCU.General_SM.CONNECTING),
        msg="HVSCU should be in CONNECTNG state!",
    )

    check(
        HVSCU.check_open_contactors,
        msg = "HVSCU should have the contactors open in connecting State"
    )
    # Now HVSCU is on Connecting state

    hvscu.connect_gui()
    completes(
        wait_until_true(hvscu.is_state),
        args=(HVSCU.General_SM.OPERATIONAL),
        before=seconds(1),
        msg="HVSCU should be in OPERATIONAL state!",
    )

    # Now HVSCU is on Operational state

    open_contactors = hvscu.packets.serialize_packet(901)
    close_contactors = hvscu.packets.serialize_packet(900)
    hvscu.transmit(open_contactors)
    
    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(1),
        msg="As I sent open Contactors should be still open the contactors",
    )
    
    #now let's send close_contactors
    hvscu.transmit(close_contactors)

    completes(
        wait_until_true(hvscu.check_precharge),
        before=seconds(1),
        msg="HVSCU should be in precharge after sending open Contactors",
    )
    #let's send in the middle of precharge open contactors
    
    hvscu.transmit(open_contactors)
    
    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(0.3),
        msg="As I sent open Contactors should be open the contactors",
    )

    #now let's check all the steps of close_contactors
    hvscu.transmit(close_contactors)

    #check precharge
    completes(
        wait_until_true(hvscu.check_precharge),
        before=seconds(1),
        msg="HVSCU should be in precharge after sending open Contactors",
    )
    #check charge I use a big interval so there is no problem 
    completes(
        wait_until_true(hvscu.check_charge),
        after=seconds(1.5),
        before = seconds(6),
        msg="HVSCU should be in precharge after sending open Contactors",
    )
    
    
    #let's check again that once in charge we can change to open contactors
    hvscu.transmit(open_contactors)
    
    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(0.3),
        msg="As I sent open Contactors should be open the contactors",
    )
    
    #send close again to check that in fault the contactors become open
    hvscu.transmit(close_contactors)

    #check precharge
    completes(
        wait_until_true(hvscu.check_precharge),
        before=seconds(1),
        msg="HVSCU should be in precharge after sending open Contactors",
    )
    #finally let's disconnect from the gui and check that goes to open contactors and the state machine changes to FAULT
    
    hvscu.disconnect_gui()

    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(0.1), 
        msg="As I sent open Contactors should be open the contactors in less than 100 ms for safety reasons said by rules & regulations",
    )
    #also let's check the state of the HVSCU state machine
    completes(
        hvscu.is_state,
        args=(HVSCU.StateMachine.GENERAL, HVSCU.General_SM.FAULT),
        before = seconds(0.1) #let's use 0.1
        msg="HVSCU should be in FAULT state!",
    )
    #finished the test succesfully

runner.run()  # Runs the tests, do not delete!
