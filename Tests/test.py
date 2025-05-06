import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), "VirtualMCU", "src"))

from HVSCU import HVSCU
from runner import runner
from vmcu.assertions import *


@runner.test()
def HVSCU_Contactors_test():
    hvscu = HVSCU()
    check(
        hvscu.is_state,
        args=(hvscu.General_SM.CONNECTING,),
        msg="HVSCU should be in CONNECTNG state!",
    )
    print("HVSCU is in Connecting State")
    print("Let's check contactors")
    check(
        hvscu.check_open_contactors,
        msg = "HVSCU should have the contactors open in connecting State",
    )
    print("HVSCU has open contactors in connecting mode")
    # Now HVSCU is on Connecting state
    print("Connect to the HVSCU")
    hvscu.connect_gui()
    print("Connection established")    
    completes(
        wait_until_true(hvscu.is_state_operational,),
        before=seconds(1),
        msg="HVSCU should be in OPERATIONAL state!",
    )

    # Now HVSCU is on Operational state

    open_contactors = hvscu.packets.serialize_packet(901)
    close_contactors = hvscu.packets.serialize_packet(900)
    hvscu.transmit(open_contactors)
    print("Transmit open contactors")
    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(1),
        msg="As I sent open Contactors should be still open the contactors",
    )
    
    #now let's send close_contactors
    hvscu.transmit(close_contactors)
    print("Transmit Close contactors")
    completes(
        wait_until_true(hvscu.check_precharge),
        before=seconds(1),
        msg="HVSCU should be in precharge after sending Close Contactors",
    )
    #let's send in the middle of precharge open contactors
    
    hvscu.transmit(open_contactors)
    print("Send open contactors in the middle of close contactor routine")
    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(0.3),
        msg="As I sent open Contactors should be open the contactors",
    )

    #now let's check all the steps of close_contactors
    hvscu.transmit(close_contactors)
    print("Close contactors again and check the whole routine")
    #check precharge
    completes(
        wait_until_true(hvscu.check_precharge),
        before=seconds(1),
        msg="HVSCU should be in precharge after sending Close Contactors",
    )
    print("check Contactors precharge Done")
    #check charge I use a big interval so there is no problem 
    time.sleep(5)
    completes(
        wait_until_true(hvscu.check_precharge),
       # after=seconds(4),
        before = seconds(2),
        msg="HVSCU should be in charge after sending Close Contactors",
    )
    print("Check contactors charge Done")
    #let's check again that once in charge we can change to open contactors
    hvscu.transmit(open_contactors)
    print("Send now again open contactors and check")
    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(0.3),
        msg="As I sent open Contactors should be open the contactors",
    )
    print("Check open contactors done")
    #send close again to check that in fault the contactors become open
    
    #check precharge
    completes(
        wait_until_true(hvscu.is_state_operational,),
        before=seconds(1),
        msg="HVSCU should be in OPERATIONAL state!",
    )
    hvscu.transmit(close_contactors)
    print("Check now the change from close to open in fault mode")
    
    completes(
        wait_until_true(hvscu.check_precharge),
        before=seconds(1),
        msg="HVSCU should be in precharge after sending close Contactors",
    )
    print("check Contactors precharge Done")
    #finally let's disconnect from the gui and check that goes to open contactors and the state machine changes to FAULT
    
    hvscu.disconnect_gui()
    print("Disconnected from the GUI")
    completes(
        wait_until_true(hvscu.check_open_contactors),
        before=seconds(0.1), 
        msg="As I sent open Contactors should be open the contactors in less than 100 ms for safety reasons said by rules & regulations",
    )
    print("Open contactors Done")
    #also let's check the state of the HVSCU state machine
    check(
        hvscu.is_state,
        args=(hvscu.General_SM.FAULT,),
        msg="HVSCU should be in FAULT state!",
    )
    print("Fault State Done")
    #finished the test succesfully

runner.run()  # Runs the tests, do not delete!
