import re
from collections import defaultdict

kHeapAddress = 0x400000
kRGBMax = 255

def parseDynamicLog(log_file):
  counter_dict = defaultdict(int)
  with open(log_file, 'r') as f:
    for line in f:
      match = re.match(r'(\d+)\s+\'.*?\'\s+counter:\s+(\d+)', line)
      if match:
        node_id, count = match.groups()
        counter_dict[node_id] = max(counter_dict[node_id], int(count))
  return counter_dict

def shouldNotChangeColor(node_id, label):
  if int(node_id) < kHeapAddress: # constants
    return True

  label = label.strip()

  if label.startswith('%'):
    return True

  reversed_label = label[::-1]

  i = 0
  while ((i < len(reversed_label)) and (reversed_label[i].isdigit())):
    i += 1

  if ((i > 0) and (i < len(reversed_label)) and (reversed_label[i] == '%')):
    return True

  return False

def processDotFile(dot_file, counter_dict, output_file):
  if not counter_dict:
    max_count = 1
  else:
    max_count = max(max(counter_dict.values()), 1)

  with open(dot_file, 'r') as f_in, open(output_file, 'w') as f_out:
    for line in f_in:
      if (('label=' in line) and ('[' in line) and (']' in line)):
        node_match = re.search(r'(\d+)\s*\[label="(.+?)"(.*?)\]', line, re.DOTALL)

        if (node_match):
          node_id, label, attrs = node_match.groups()

          if (shouldNotChangeColor(node_id, label)):
            f_out.write(line)
            continue

          count = counter_dict.get(node_id, 0)
          escaped_label = label.replace('"', '\\"')
          normalized = count / max_count
          r = int(kRGBMax * normalized)
          g = int(kRGBMax * (1 - normalized))
          hex_color = f"#{r:02x}{g:02x}00"

          new_line = f'{node_id} [label="{escaped_label}"{attrs}, fillcolor="{hex_color}", style=filled]\n'
          line = new_line

      f_out.write(line)

if (__name__ == "__main__"):
  dot_file = "assets/graph.dot"
  log_file = "assets/dynamic.log"
  output_file = "assets/colored_graph.dot"

  counter_data = parseDynamicLog(log_file)
  processDotFile(dot_file, counter_data, output_file)
