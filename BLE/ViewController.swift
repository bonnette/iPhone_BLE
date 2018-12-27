//
//  ViewController.swift
//  BLE
//
//  Created by Bear Cahill on 2/13/18.
//  Copyright © 2018 Bear Cahill. All rights reserved.
//

import UIKit
import CoreBluetooth

let arduinoSvc = CBUUID.init(string: "DF01")
let arduinoLEDchar = CBUUID.init(string: "DF02")
let arduinoLEDstate = CBUUID.init(string: "DF03")
var LedSendChar: CBCharacteristic!
var LedReadState: CBCharacteristic!
var savedPeripheral: CBPeripheral?
var x = false
var led = false

class ViewController: UIViewController, CBCentralManagerDelegate, CBPeripheralDelegate {

    @IBAction func LedStateButton(_ sender: UIButton) {
        print("Button Clicked")
        if x {
        savedPeripheral!.writeValue(Data.init(bytes: [42]), for: LedSendChar, type: CBCharacteristicWriteType.withResponse)
            x = false
        }else {
         savedPeripheral!.writeValue(Data.init(bytes: [41]), for: LedSendChar, type: CBCharacteristicWriteType.withResponse)
            x = true
        }
    }
    
    @IBOutlet weak var stateLabel: UILabel!
    
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == CBManagerState.poweredOn {
            central.scanForPeripherals(withServices: nil, options: nil)
            print ("scanning...")
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if peripheral.name?.contains("ESP") == true {
            savedPeripheral = peripheral // place the peripheral in global for use in button
            print ("The pName is ", peripheral.name ?? "no name")
            centralManager.stopScan()
            print ("The Advert data is ", advertisementData)
            central.connect(peripheral, options: nil)
            myPeripheral = peripheral
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        central.scanForPeripherals(withServices: nil, options: nil)
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print ("Connected to = ", [peripheral.name])
        peripheral.discoverServices(nil)
        peripheral.delegate = self
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if let services = peripheral.services {
            for svc in services {
                if svc.uuid == arduinoSvc {
                    print ("We have found ", svc.uuid.uuidString)
                    peripheral.discoverCharacteristics(nil, for: svc)
                }
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let chars = service.characteristics {
            for char in chars {
                print (char.uuid.uuidString)
                if char.uuid == arduinoLEDchar {
                    LedSendChar = char // place the charateristic in global for use in button

                }else if char.properties.contains(CBCharacteristicProperties.notify) {
                        print("read and notify Characteristic \(char.uuid.uuidString)")
                    LedReadState = char // Place read charateristic in global for use later
                    peripheral.setNotifyValue(true, for: char)
                }
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {

            if characteristic.uuid == arduinoLEDstate {
                let s = characteristic.value![0] // Get the ascii value of the value from the BLE device
                let pz = Character(UnicodeScalar(s)) // Convert ascii to a charater
                print (pz)
                if pz == "N" {
                    led = true
                    print("LED is on")
                    stateLabel.text = "ON"
                    stateLabel.textColor = UIColor.red
                    stateLabel.textAlignment = .center
                }else {
                    led = false
                    print("LED is off")
                    stateLabel.text = "OFF"
                    stateLabel.textColor = UIColor.blue
                    stateLabel.textAlignment = .center
                }
            }
        
    }

    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        print ("wrote value")
    }
    
    var centralManager : CBCentralManager!
    var myPeripheral : CBPeripheral?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        centralManager = CBCentralManager.init(delegate: self, queue: nil)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

