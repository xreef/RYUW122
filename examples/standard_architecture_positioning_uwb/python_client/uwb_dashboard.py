import customtkinter as ctk
import tkinter as tk
from tkinter import ttk
import json
import time
import math
import threading
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.patches as patches
import paho.mqtt.client as mqtt
from datetime import datetime

# --- Configuration ---
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_USER = "position"
MQTT_PASSWORD = "position"
MQTT_TOPIC_DISTANCE = "uwb/trilateration/distance"
MQTT_TOPIC_POLL_REQUEST = "uwb/trilateration/poll_request"

# Anchor Configuration (Coordinates in meters)
# 0,0 is Top-Left. Y increases downwards.
ANCHORS = {
    "MA":  {"x": 0.0, "y": 0.0, "color": "#FF5733"},   # Top-Left
    "SA1": {"x": 5.3, "y": 0.0, "color": "#33FF57"},   # Top-Right
    "SA2": {"x": 5.3, "y": 3.65, "color": "#3357FF"}   # Bottom-Right
}

# List of Tags
TAG_ADDRESSES = ["T1T1T1T1"]

# --- App Class ---
class UWBDashboard(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("UWB Real-Time Positioning System")
        self.geometry("1200x800")
        ctk.set_appearance_mode("Dark")  # Modes: "System" (standard), "Dark", "Light"
        ctk.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"

        # Data State
        # Structure: { tag_id: { 'distances': {aid: dist}, 'rssi': {aid: rssi}, 'position': (x,y), 'history_x': [], 'history_y': [] } }
        self.tags = {}
        self.max_history = 50
        
        # Initialize data structures for configured tags
        colors = ['red', 'yellow', 'cyan', 'magenta', 'orange']
        for i, tag in enumerate(TAG_ADDRESSES):
            self.tags[tag] = {
                "distances": {},
                "rssi": {},
                "position": None,
                "history_x": [],
                "history_y": [],
                "color": colors[i % len(colors)]
            }

        self.last_update_time = time.time()

        # Layout Configuration
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # --- Sidebar (Controls & Info) ---
        self.sidebar_frame = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(4, weight=1)

        self.logo_label = ctk.CTkLabel(self.sidebar_frame, text="UWB Dashboard", font=ctk.CTkFont(size=20, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 10))

        self.btn_poll = ctk.CTkButton(self.sidebar_frame, text="Send Poll Request", command=self.send_poll_request)
        self.btn_poll.grid(row=1, column=0, padx=20, pady=10)

        self.auto_poll_var = ctk.BooleanVar(value=False)
        self.chk_auto_poll = ctk.CTkSwitch(self.sidebar_frame, text="Auto Poll (2s)", variable=self.auto_poll_var, command=self.toggle_auto_poll)
        self.chk_auto_poll.grid(row=2, column=0, padx=20, pady=10)

        self.lbl_status = ctk.CTkLabel(self.sidebar_frame, text="Status: Disconnected", text_color="gray")
        self.lbl_status.grid(row=3, column=0, padx=20, pady=10)

        # Anchor Status List
        self.anchor_status_frame = ctk.CTkScrollableFrame(self.sidebar_frame, label_text="Anchor Status")
        self.anchor_status_frame.grid(row=4, column=0, padx=20, pady=10, sticky="nsew")
        self.anchor_labels = {}

        for aid in ANCHORS:
            lbl = ctk.CTkLabel(self.anchor_status_frame, text=f"{aid}: -- m (RSSI: --)", anchor="w")
            lbl.pack(fill="x", padx=5, pady=2)
            self.anchor_labels[aid] = lbl

        # --- Main Content Area ---
        self.main_frame = ctk.CTkFrame(self, corner_radius=0, fg_color="transparent")
        self.main_frame.grid(row=0, column=1, sticky="nsew")
        self.main_frame.grid_rowconfigure(0, weight=1)
        self.main_frame.grid_columnconfigure(0, weight=1)

        # Matplotlib Figure
        self.fig = Figure(figsize=(5, 4), dpi=100, facecolor='#2b2b2b') # Dark background for plot
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor('#2b2b2b')
        self.ax.tick_params(colors='white')
        self.ax.xaxis.label.set_color('white')
        self.ax.yaxis.label.set_color('white')
        self.ax.spines['bottom'].set_color('white')
        self.ax.spines['top'].set_color('white') 
        self.ax.spines['right'].set_color('white')
        self.ax.spines['left'].set_color('white')

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.main_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().grid(row=0, column=0, sticky="nsew", padx=10, pady=10)

        # --- MQTT Setup ---
        # Use CallbackAPIVersion.VERSION2 to avoid deprecation warning and handle callbacks correctly
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        
        # Set Username and Password
        self.client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
        
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        
        self.mqtt_thread = threading.Thread(target=self.start_mqtt)
        self.mqtt_thread.daemon = True
        self.mqtt_thread.start()

        # --- Auto Poll Timer ---
        self.auto_poll_timer = None

        # Initial Plot
        self.update_plot()

    def start_mqtt(self):
        try:
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.client.loop_forever()
        except Exception as e:
            print(f"MQTT Connection Error: {e}")
            self.after(0, lambda: self.lbl_status.configure(text="Status: MQTT Error", text_color="red"))

    def on_connect(self, client, userdata, flags, reason_code, properties):
        print(f"Connected with result code {reason_code}")
        if reason_code == 0:
            client.subscribe(MQTT_TOPIC_DISTANCE)
            client.subscribe(MQTT_TOPIC_POLL_REQUEST)
            self.after(0, lambda: self.lbl_status.configure(text="Status: Connected", text_color="green"))
        else:
            print(f"Connection failed with code {reason_code}")
            if reason_code == 4:
                print("Error: Broker rejected connection (Bad Username/Password).")
            self.after(0, lambda: self.lbl_status.configure(text=f"Status: Conn Error {reason_code}", text_color="red"))

    def on_message(self, client, userdata, msg):
        try:
            payload_str = msg.payload.decode()
            payload = json.loads(payload_str)
            topic = msg.topic
            
            if topic == MQTT_TOPIC_DISTANCE:
                self.handle_distance(payload)
            elif topic == MQTT_TOPIC_POLL_REQUEST:
                # Optional: Visual feedback for poll requests
                pass
                
        except Exception as e:
            print(f"Error parsing message: {e}")

    def handle_distance(self, data):
        anchor_id = data.get("anchor")
        distance_cm = data.get("distance_cm")
        rssi = data.get("rssi")
        
        # Try to get tag ID from data, default to first tag if missing (for backward compatibility)
        tag_id = data.get("tag", data.get("tag_address", TAG_ADDRESSES[0]))
        
        if tag_id not in self.tags:
            # If we receive data for an unknown tag, we could add it dynamically or ignore.
            # For now, let's ignore or map to first if it's the only one.
            if len(TAG_ADDRESSES) == 1:
                tag_id = TAG_ADDRESSES[0]
            else:
                return # Ignore unknown tags

        if anchor_id in ANCHORS:
            dist_m = distance_cm / 100.0
            
            # Update specific tag data
            self.tags[tag_id]["distances"][anchor_id] = dist_m
            self.tags[tag_id]["rssi"][anchor_id] = rssi
            
            # Update Anchor Label (Just for the first tag or the one being updated)
            # To avoid flickering with multiple tags, maybe only update for the first tag in list
            if tag_id == TAG_ADDRESSES[0]:
                self.after(0, lambda a=anchor_id, d=dist_m, r=rssi: self.update_anchor_label(a, d, r))

            # Try to calculate position for this tag
            self.try_trilateration(tag_id)
            
            # Refresh plot
            self.after(0, self.update_plot)

    def update_anchor_label(self, anchor_id, distance, rssi):
        if anchor_id in self.anchor_labels:
            self.anchor_labels[anchor_id].configure(text=f"{anchor_id}: {distance:.2f} m (RSSI: {rssi} dBm)")

    def send_poll_request(self):
        poll_id = int(time.time() * 1000)
        
        # Send poll request for each tag
        for tag in TAG_ADDRESSES:
            payload = {
                "poll_id": poll_id,
                "tag_address": tag
            }
            try:
                self.client.publish(MQTT_TOPIC_POLL_REQUEST, json.dumps(payload))
                print(f"Sent Poll Request for {tag}: {payload}")
            except Exception as e:
                print(f"Error publishing: {e}")

    def toggle_auto_poll(self):
        if self.auto_poll_var.get():
            self.schedule_poll()
        else:
            if self.auto_poll_timer:
                self.after_cancel(self.auto_poll_timer)
                self.auto_poll_timer = None

    def schedule_poll(self):
        if self.auto_poll_var.get():
            self.send_poll_request()
            self.auto_poll_timer = self.after(2000, self.schedule_poll)

    def try_trilateration(self, tag_id):
        tag_data = self.tags[tag_id]
        distances = tag_data["distances"]
        
        valid_anchors = []
        for aid, pos in ANCHORS.items():
            if aid in distances and distances[aid] > 0:
                valid_anchors.append((pos['x'], pos['y'], distances[aid]))
        
        if len(valid_anchors) < 3:
            return

        # Use the first 3 found
        x1, y1, r1 = valid_anchors[0]
        x2, y2, r2 = valid_anchors[1]
        x3, y3, r3 = valid_anchors[2]

        A = 2 * (x2 - x1)
        B = 2 * (y2 - y1)
        C = r1**2 - r2**2 - x1**2 + x2**2 - y1**2 + y2**2
        
        D = 2 * (x3 - x2)
        E = 2 * (y3 - y2)
        F = r2**2 - r3**2 - x2**2 + x3**2 - y2**2 + y3**2
        
        denom = A * E - B * D
        if abs(denom) < 1e-9:
            return

        x = (C * E - B * F) / denom
        y = (A * F - C * D) / denom
        
        tag_data["position"] = (x, y)
        
        # Update History
        tag_data["history_x"].append(x)
        tag_data["history_y"].append(y)
        if len(tag_data["history_x"]) > self.max_history:
            tag_data["history_x"].pop(0)
            tag_data["history_y"].pop(0)

    def update_plot(self):
        self.ax.clear()
        
        # Set Plot Limits
        all_x = [p['x'] for p in ANCHORS.values()]
        all_y = [p['y'] for p in ANCHORS.values()]
        
        padding = 1.0
        self.ax.set_xlim(min(all_x) - padding, max(all_x) + padding)
        self.ax.set_ylim(min(all_y) - padding, max(all_y) + padding)
        
        # Invert Y axis to have (0,0) at Top-Left
        self.ax.invert_yaxis()
        
        self.ax.set_aspect('equal')
        self.ax.grid(True, color='#444444', linestyle='--')
        self.ax.set_title("Live Position Map (0,0 Top-Left)", color='white')
        self.ax.set_xlabel("X (meters)", color='white')
        self.ax.set_ylabel("Y (meters)", color='white')

        # Draw Anchors
        for aid, pos in ANCHORS.items():
            self.ax.scatter(pos['x'], pos['y'], c=pos['color'], s=100, label=aid, edgecolors='white', zorder=5)
            self.ax.text(pos['x'], pos['y'] + 0.2, aid, color=pos['color'], ha='center', fontweight='bold')
            
            # Draw circles for the first tag only to avoid clutter, or maybe for selected tag?
            # For now, let's draw circles for the first tag in the list
            first_tag = TAG_ADDRESSES[0]
            if first_tag in self.tags:
                dists = self.tags[first_tag]["distances"]
                if aid in dists and dists[aid] > 0:
                    circle = patches.Circle((pos['x'], pos['y']), dists[aid], 
                                          fill=False, edgecolor=pos['color'], linestyle='--', alpha=0.3)
                    self.ax.add_patch(circle)

        # Draw Tags
        for tag_id, data in self.tags.items():
            # Draw History
            if data["history_x"]:
                self.ax.plot(data["history_x"], data["history_y"], color=data["color"], alpha=0.5, linewidth=1, linestyle=':')

            # Draw Current Position
            pos = data["position"]
            if pos:
                self.ax.scatter(pos[0], pos[1], c=data["color"], s=150, marker='X', label=tag_id, edgecolors='white', zorder=10)
                self.ax.text(pos[0], pos[1] - 0.3, 
                             f"{tag_id}\n({pos[0]:.2f}, {pos[1]:.2f})", 
                             color=data["color"], ha='center', fontsize=8)

        # Legend
        self.ax.legend(loc='upper right', facecolor='#333333', edgecolor='white', labelcolor='white')

        self.canvas.draw()

if __name__ == "__main__":
    app = UWBDashboard()
    app.mainloop()
