
##################################################################################################
##################################################################################################
#############################################IMPORTANT############################################
# This is a obsolete code, we are not using tpLink smart plug to controll the humidifier anymore.
##################################################################################################
##################################################################################################
##################################################################################################
from tplinkcloud import TPLinkDeviceManager

import asyncio
import json

username = 'bhartiprakashtiwary@gmail.com'
password = 'Bintern@15'

device_manager = TPLinkDeviceManager(username, password)

async def power_on_switch():
  devices = await device_manager.get_devices()
  fetch_tasks = []
  for device in devices:
    async def get_info(device):
      device_alias = device.get_alias()
      if device_alias == 'Humidifier':
        print("Attempting to power on...")
        await device.power_on()
        print("done..")

    fetch_tasks.append(get_info(device))
  await asyncio.gather(*fetch_tasks)

async def power_off_switch():
  devices = await device_manager.get_devices()
  fetch_tasks = []
  for device in devices:
    async def get_info(device):
      device_alias = device.get_alias()
      if device_alias == 'Humidifier':
        print("Attempting to power off...")
        await device.power_off()
        print("done..")

    fetch_tasks.append(get_info(device))
  await asyncio.gather(*fetch_tasks)

def power_on():
  asyncio.run(power_on_switch())

def power_off():
  asyncio.run(power_off_switch())


power_on()

