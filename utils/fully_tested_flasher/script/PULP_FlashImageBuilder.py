########################################## Images Builder ##################################

import os, sys

def FlashImageBuilder (Components):

	path = os.getcwd()
	image_path = path + "/components"
	components_list = os.listdir(image_path)

	f = open(path + "/Image.hex", 'wb+')

	for file in Components:
		if file in components_list:
			print("Adding Component: %s" % file)
			with open(image_path + "/%s" % file, 'rb') as file2app:
				for i in file2app:
					f.write(i)
			file2app.close()
	
	f.close()

# total size: 122.176 bytes
PULP_FLASH = ["ConvBNRelu0_weights.hex", "ConvDWBNRelu1_weights.hex", "ConvBNRelu2_weights.hex", "ConvDWBNRelu3_weights.hex",
				"ConvBNRelu4_weights.hex", "ConvDWBNRelu5_weights.hex", "ConvBNRelu6_weights.hex", "ConvDWBNRelu7_weights.hex",
				"ConvBNRelu8_weights.hex", "ConvDWBNRelu9_weights.hex", "inputs.hex"]

FlashImageBuilder(PULP_FLASH)