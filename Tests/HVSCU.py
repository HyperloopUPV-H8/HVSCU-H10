from vmcu.services.communications.Socket import Socket
from vmcu.shared_memory import SharedMemory
from vmcu.services.digital_out import DigitalOutService
from vmcu.pin.pinout import Pinout
from vmcu.services.communications.Packets import Packets
from enum import IntEnum
from vmcu.pin.memory import DigitalOut

class HVSCU:
    
    class General_SM(IntEnum):
        CONNECTING = 0
        OPERATIONAL = 1
        FAULT = 2

    class Contactors(IntEnum):
        HIGH = 0,
        PRECHARGE = 1,
        LOW = 2,
        DISCHARGE = 3


    def __init__(self):
        self._shm = SharedMemory("shm_gpio_HVSCU", "shm_sm_HVSCU")
        self._client = Socket("192.168.0.9", 50400, "192.168.1.7", 50500)
        packet_definitions = {
            900: [],  # Close contactors
            901: [],  # Open Contactors
        }
        self.packets = Packets(packet_definitions)
        self._enable = DigitalOutService(self._shm, Pinout.PF4)
        self._contactors = {
            HVSCU.Contactors.HIGH : DigitalOutService(self._shm,Pinout.PG12),
            HVSCU.Contactors.PRECHARGE : DigitalOutService(self._shm,Pinout.PD4),
            HVSCU.Contactors.LOW : DigitalOutService(self._shm,Pinout.PG14),
            HVSCU.Contactors.DISCHARGE : DigitalOutService(self._shm,Pinout.PF4)
        }
        print("HVSCU initialized succesfully")

    def connect_gui(self):
        self._client.connect()

    def disconnect_gui(self):
        self._client.stop()

    def is_state(self, state: int) -> bool:
        current_state = self._shm.get_state_machine_state(1)
        return current_state == state
    
    def is_state_operational(self)->bool:
        return self._shm.get_state_machine_state(1) == HVSCU.General_SM.OPERATIONAL
    
    def check_contactor(self, contactor: Contactors, state: DigitalOut.State) -> bool:
        selected_contactor = self._contactors[contactor]
        return selected_contactor.get_pin_state() == state
    
    def check_precharge(self) ->bool:
        return (
            self._contactors[HVSCU.Contactors.HIGH].get_pin_state() ==  DigitalOut.State.Low and 
            self._contactors[HVSCU.Contactors.PRECHARGE].get_pin_state() ==  DigitalOut.State.High and 
            self._contactors[HVSCU.Contactors.LOW].get_pin_state() ==  DigitalOut.State.High and 
            self._contactors[HVSCU.Contactors.DISCHARGE].get_pin_state() ==  DigitalOut.State.High
        )
        
        
    def check_charge(self)->bool:
           return (
            self._contactors[HVSCU.Contactors.HIGH].get_pin_state() ==  DigitalOut.State.High and 
            self._contactors[HVSCU.Contactors.PRECHARGE].get_pin_state() ==  DigitalOut.State.Low and 
            self._contactors[HVSCU.Contactors.LOW].get_pin_state() ==  DigitalOut.State.High and 
            self._contactors[HVSCU.Contactors.DISCHARGE].get_pin_state() ==  DigitalOut.State.High
        )
    def check_open_contactors(self)->bool:
          return (
            self._contactors[HVSCU.Contactors.HIGH].get_pin_state() ==  DigitalOut.State.Low and 
            self._contactors[HVSCU.Contactors.PRECHARGE].get_pin_state() ==  DigitalOut.State.Low and 
            self._contactors[HVSCU.Contactors.LOW].get_pin_state() ==  DigitalOut.State.Low and 
            self._contactors[HVSCU.Contactors.DISCHARGE].get_pin_state() ==  DigitalOut.State.Low
        )
        
        

    def transmit(self, packet):
        self._client.transmit(packet)
        
    def __del__(self):
        self._client.stop()
        
