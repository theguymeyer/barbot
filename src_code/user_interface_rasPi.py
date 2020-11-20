import tkinter as tk
import serial
import time

class HomePage(tk.Frame):
	
	drinks = {'r': "Rum & Coke", 'v': "Vodka & Cran", 'm': "Mojito"}
	
	def __init__(self, master=None):
		super().__init__(master)
		self.master = master
		self.pack()
		
		'''
		self.create_widgets()
		
	def create_widgets(self):
		self.RumCoke = tk.Button(self, height=4, width=10)
		self.RumCoke["text"] = "Rum & Coke"
		self.RumCoke["command"] = self.rum_coke
		self.RumCoke.pack(side="top")
		
		self.VodkaCran = tk.Button(self, height=4, width=10)
		self.VodkaCran["text"] = "Vodka Cran"
		self.VodkaCran["command"] = self.vodka_cran
		self.VodkaCran.pack(side="top")
		
		self.Mojito = tk.Button(self, height=4, width=10)
		self.Mojito["text"] = "Mojito"
		self.Mojito["command"] = self.mojito
		self.Mojito.pack(side="top")
		'''
		
def rum_coke(serialCmd):
	print("Making Rum & Coke")
	# send signal to main_controller
	serialCmd.write(str.encode('r'))
	# TODO disable buttons
	
	# wait for finished drink signal
	
	serialCmd.flush()
	print(serialCmd.read())
	serialCmd.flush()
	
	while serialCmd.inWaiting():
		print(serialCmd.read())
	
	print("Finished Rum & Coke")


def vodka_cran(serialCmd):
	print("Making Vodka Cran")
	# send signal to main_controller
	serialCmd.write(str.encode('v'))
	# TODO disable buttons
	
	# wait for finished drink signal
	
	serialCmd.flush()
	print(serialCmd.read())
	serialCmd.flush()
	
	while serialCmd.inWaiting():
		print(serialCmd.read())
	
	print("Finished Vodka Cran")


def mojito(serialCmd):
	print("Making Mojito")
	# send signal to main_controller
	serialCmd.write(str.encode('m'))
	# TODO disable buttons
	
	# wait for finished drink signal
	
	serialCmd.flush()
	print(serialCmd.read())
	serialCmd.flush()
	
	while serialCmd.inWaiting():
		print(serialCmd.read())
	
	print("Finished Mojito")
				


	
# connect arduino
ser = serial.Serial(
	port = '/dev/ttyACM0',
	baudrate = 9600
)	
	
root = tk.Tk()
app = HomePage(master=root)

rumcokeTxt = "Rum & Coke\n\n1 part rum\n3 parts coke"
vodkacranTxt = "Vodka Cran\n\nequal parts Vodka\nCranberry Juice"
mojitoTxt = "Mojito\n\n1 part rum\n2 parts LemonMint Juice"

RumCoke = tk.Button(app, height=6, width=30, text = rumcokeTxt, command = lambda: rum_coke(ser))
RumCoke.pack(side="top")

VodkaCran = tk.Button(app, height=6, width=30, text = vodkacranTxt, command = lambda: vodka_cran(ser))
VodkaCran.pack(side="top")

Mojito = tk.Button(app, height=6, width=30, text = mojitoTxt, command = lambda: mojito(ser))
Mojito.pack(side="top")


app.mainloop()
