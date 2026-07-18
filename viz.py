import subprocess
import re
import sys
import os
import pygame
import math

# --- 1. ARGUMENTS CONFIGURATION ---
CDRS = 5
BRN = 100
TC = 100
TD = 100
TR = 100
RQC = 2
CLDWN = 0
SCH = "fifo"

ARGS = [CDRS, BRN, TC, TD, TR, RQC, CLDWN, SCH]

# --- GRAPHICS CONFIGURATION ---
WIDTH, HEIGHT = 1100, 850
FPS = 30

C_WHITE      = (255, 255, 255)
C_BLACK      = (0, 0, 0)
C_BG_BLUE    = (15, 23, 42)
C_TEXT_LINE  = (56, 189, 248)
C_DONGLE_IDLE= (148, 163, 184)  # Gray
C_DONGLE_BUSY= (239, 68, 68)    # Red when locked
C_IDLE       = (100, 116, 139)
C_GREEN      = (34, 197, 94)    # Compiling
C_ORANGE     = (249, 115, 22)   # Debugging
C_PURPLE     = (168, 85, 247)   # Refactoring
C_BURNT_OUT  = (239, 68, 68)    # Red block for burnout
C_TIME_GOLD  = (250, 204, 21)   # Center clock text color

CODER_RADIUS = 45
DONGLE_SIZE = 25

def strip_ansi_codes(text):
    ansi_pattern = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_pattern.sub('', text).replace('*', '')

def run_and_parse_simulation():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    exec_path = os.path.join(script_dir, "codexion")
    if not os.path.exists(exec_path):
        exec_path = "./codexion"

    cmd = [exec_path] + [str(arg) for arg in ARGS]
    try:
        process = subprocess.run(cmd, capture_output=True, text=True, check=True)
        raw_lines = process.stdout.splitlines()
    except Exception as e:
        print(f"[ERROR] Failed running binary: {e}")
        sys.exit(1)

    events = []
    log_pattern = re.compile(r"^\s*(\d+)\s*\|\s*(\d+)\s+(.+)$")

    for line in raw_lines:
        cleaned = strip_ansi_codes(line.strip())
        if not cleaned or cleaned.startswith("[+") or cleaned.startswith("[-"):
            continue
        match = log_pattern.match(cleaned)
        if match:
            timestamp, coder_id, action = match.groups()
            events.append({
                "time": int(timestamp),
                "coder_id": int(coder_id),
                "state": action.strip(),
                "raw": cleaned
            })
    return events

def get_state_details(state_text):
    text = state_text.lower()
    if "burnt out" in text or "burned out" in text or "terminated" in text:
        return {"color": C_BURNT_OUT, "label": "BURNT OUT", "fill": 0, "tag": "burnt"}
    elif "dongle" in text:
        return {"color": C_WHITE, "label": "HOLD DGL", "fill": 3, "tag": "dongle"}
    elif "compiling" in text:
        return {"color": C_GREEN, "label": "COMPILE", "fill": 0, "tag": "compile"}
    elif "debugging" in text:
        return {"color": C_ORANGE, "label": "DEBUG", "fill": 0, "tag": "debug"}
    elif "refactoring" in text:
        return {"color": C_PURPLE, "label": "REFACTOR", "fill": 0, "tag": "refactor"}
    else:
        return {"color": C_IDLE, "label": "IDLE", "fill": 0, "tag": "idle"}

def precompute_history(events, num_coders):
    history = []
    curr_coder_states = {i: "IDLE" for i in range(1, num_coders + 1)}
    coder_dongle_counts = {i: 0 for i in range(1, num_coders + 1)}

    for ev in events:
        c_id = ev["coder_id"]
        state = ev["state"]

        if "has taken a dongle" in state.lower():
            coder_dongle_counts[c_id] = min(2, coder_dongle_counts[c_id] + 1)
            curr_coder_states[c_id] = "has taken a dongle"
        elif "compiling" in state.lower():
            curr_coder_states[c_id] = "is compiling"
        elif "debugging" in state.lower() or "refactoring" in state.lower():
            coder_dongle_counts[c_id] = 0
            curr_coder_states[c_id] = state
        elif "burnt out" in state.lower() or "burned out" in state.lower():
            coder_dongle_counts[c_id] = 0
            curr_coder_states[c_id] = "burnt out"

        dongle_occupancy = [False] * num_coders
        active_links = []

        for idx in range(1, num_coders + 1):
            d_count = coder_dongle_counts[idx]
            left_d = idx - 1
            right_d = (idx - 2) % num_coders

            if d_count >= 1:
                dongle_occupancy[left_d] = True
                active_links.append((idx, left_d))
            if d_count == 2:
                dongle_occupancy[right_d] = True
                active_links.append((idx, right_d))

        history.append({
            "time": ev["time"],
            "coder_states": curr_coder_states.copy(),
            "dongle_states": dongle_occupancy.copy(),
            "links": list(active_links),
            "raw": ev["raw"]
        })

    return history

def main():
    events = run_and_parse_simulation()
    if not events:
        print("[ERROR] No logs found.")
        return

    history = precompute_history(events, CDRS)
    total_steps = len(history)

    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Codexion Bidirectional Visualizer")
    clock = pygame.time.Clock()

    # Fonts
    f_huge  = pygame.font.SysFont("Impact", 54)
    f_big   = pygame.font.SysFont("Arial", 24, bold=True)
    f_mid   = pygame.font.SysFont("Arial", 16, bold=True)
    f_small = pygame.font.SysFont("Arial", 12, bold=True)
    f_log   = pygame.font.SysFont("Courier New", 18, bold=True)

    step_idx = -1
    CX, CY = WIDTH // 2, HEIGHT // 2
    CODER_RING_RADIUS = 270
    DONGLE_RING_RADIUS = 155

    coder_coords = {}
    for i in range(CDRS):
        c_id = i + 1
        angle = (2 * math.pi * i) / CDRS - math.pi / 2
        coder_coords[c_id] = (int(CX + CODER_RING_RADIUS * math.cos(angle)), int(CY + CODER_RING_RADIUS * math.sin(angle)))

    dongle_coords = []
    for i in range(CDRS):
        angle = (2 * math.pi * i) / CDRS - math.pi / 2 - (math.pi / CDRS)
        dongle_coords.append((int(CX + DONGLE_RING_RADIUS * math.cos(angle)), int(CY + DONGLE_RING_RADIUS * math.sin(angle))))

    running = True
    while running:
        screen.fill(C_BG_BLUE)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RIGHT:
                    if step_idx < total_steps - 1:
                        step_idx += 1
                elif event.key == pygame.K_LEFT:
                    if step_idx >= 0:
                        step_idx -= 1

        current_step_data = history[step_idx] if step_idx >= 0 else {
            "time": 0,
            "coder_states": {i: "IDLE" for i in range(1, CDRS + 1)},
            "dongle_states": [False] * CDRS,
            "links": [],
            "raw": "[Press RIGHT ARROW to begin timeline simulation]"
        }

        # 1. BIG CENTER TIMESTAMP TEXT DISPLAY
        time_str = f"{current_step_data['time']} ms"
        time_surf = f_huge.render(time_str, True, C_TIME_GOLD)
        screen.blit(time_surf, (CX - time_surf.get_width() // 2, CY - time_surf.get_height() // 2))

        # 2. DRAW ALLOCATION LINKS (Lines)
        for c_id, d_idx in current_step_data["links"]:
            cx, cy = coder_coords[c_id]
            dx, dy = dongle_coords[d_idx]
            state_text = current_step_data["coder_states"][c_id]
            line_color = get_state_details(state_text)["color"]
            pygame.draw.line(screen, line_color, (cx, cy), (dx, dy), 4)

        # 3. DRAW DONGLES
        for idx, (dx, dy) in enumerate(dongle_coords):
            is_busy = current_step_data["dongle_states"][idx]
            d_color = C_DONGLE_BUSY if is_busy else C_DONGLE_IDLE

            pygame.draw.rect(screen, d_color, (dx - DONGLE_SIZE//2, dy - DONGLE_SIZE//2, DONGLE_SIZE, DONGLE_SIZE))
            pygame.draw.rect(screen, C_WHITE, (dx - DONGLE_SIZE//2, dy - DONGLE_SIZE//2, DONGLE_SIZE, DONGLE_SIZE), 1)

            d_lbl = f_small.render(f"D{idx+1}", True, C_BLACK if not is_busy else C_WHITE)
            screen.blit(d_lbl, (dx - d_lbl.get_width()//2, dy - d_lbl.get_height()//2))

        # 4. DRAW CODERS
        for c_id, (cx, cy) in coder_coords.items():
            state_text = current_step_data["coder_states"][c_id]
            visual = get_state_details(state_text)

            pygame.draw.circle(screen, visual["color"], (cx, cy), CODER_RADIUS, visual["fill"])
            pygame.draw.circle(screen, C_WHITE, (cx, cy), CODER_RADIUS, 3)

            id_surf = f_mid.render(f"Coder {c_id}", True, C_WHITE if visual["fill"] == 0 else visual["color"])
            screen.blit(id_surf, (cx - id_surf.get_width() // 2, cy - 12))

            lbl_surf = f_small.render(visual["label"], True, C_BLACK if visual["fill"] == 0 else C_WHITE)
            screen.blit(lbl_surf, (cx - lbl_surf.get_width() // 2, cy + 10))

        # UI Headers Info Bar
        title_surf = f_big.render("CODEXION CONCURRENCY SIMULATION", True, C_WHITE)
        screen.blit(title_surf, (CX - title_surf.get_width() // 2, 25))

        info_str = f"Step: {step_idx + 1} / {total_steps}  |  Controls: [<-] Backwards  [->] Forwards"
        info_surf = f_small.render(info_str, True, C_DONGLE_IDLE)
        screen.blit(info_surf, (CX - info_surf.get_width() // 2, 60))

        # Bottom Action Status Box
        pygame.draw.rect(screen, C_BLACK, (40, HEIGHT - 90, WIDTH - 80, 60), border_radius=8)
        log_surf = f_log.render(f">> {current_step_data['raw']}", True, C_TEXT_LINE)
        screen.blit(log_surf, (60, HEIGHT - 72))

        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
