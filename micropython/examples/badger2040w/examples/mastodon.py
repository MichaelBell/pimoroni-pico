import badger2040w as badger2040
import urequests
import time
import gc
import re

# Secrets should contain WIFI_SSID, WIFI_PASSWORD, mastodon_server and mastodon_token
# If mastodon_token is empty then the public timeline of the server will be used.
from secrets import mastodon_server, mastodon_token

display = badger2040.Badger2040W()
display.led(128)
display.set_update_speed(badger2040.UPDATE_NORMAL)
display.set_font("bitmap8")

# Connects to the wireless network. Ensure you have entered your details in WIFI_CONFIG.py :).
display.connect()


def process_html(html):
    def tag_replace(m):
        tag = m.group(1)
        if tag in ("/p", "br", "br/"):
            return "\n"
        return ""

    def ent_replace(m):
        ent = m.group(1)
        mapping = {
            # Spported
            'amp': '&',
            'gt': '>',
            'lt': '<',
            'quot': '"',
            'copy': '©',

            # Unsupported
            'reg': '°',
            'trade': '°',
            'ldquo': '"',
            'rdquo': '"',
            'lsquo': "'",
            'rsquo': "'"
        }
        if ent in mapping:
            return mapping[ent]
        return "&"

    html = re.sub(r'<([^>]*)>', tag_replace, html.replace("\n", ""))
    return re.sub(r'&([^;]*);', ent_replace, html)


def draw_toot(toot):
    display.set_pen(15)
    display.clear()

    display.set_pen(0)
    account = toot["account"]
    display.set_font("bitmap14_outline")
    display_name = "{display_name} ".format(**account)
    name_width = display.measure_text(display_name, 1)
    display.text(display_name, 0, 0, scale=1)

    display.set_font("bitmap8")
    display.text("@{acct}".format(**account), name_width, 5, scale=1)

    display.set_font("bitmap14_outline")
    content = process_html(toot["content"]).split('\n')

    y = 18
    scale = 1
    line_space = 14

    for line in content:
        while display.measure_text(line, scale) > 290:
            split_line = line.split(" ")
            i = 1
            this_line = split_line[0]

            while i < len(split_line) and display.measure_text(this_line + " " + split_line[i], scale) <= 290:
                this_line += " " + split_line[i]
                i += 1

            display.text(this_line, 4, y, scale=scale)
            y += line_space
            if y > 127 - line_space:
                break

            line = " ".join(split_line[i:])

        if y > 127 - line_space:
            break

        display.text(line, 4, y, scale=scale)
        y += line_space
        if y > 127 - line_space:
            break

    display.update()


# Setup Mastodon query
mastodon_api = "https://%s/api/" % mastodon_server
if len(mastodon_token) > 1:
    mastodon_headers = {'Authorization': 'Bearer %s' % mastodon_token}
    timeline_url_base = mastodon_api + "/v1/timelines/home?limit=1"
else:
    mastodon_headers = {}
    timeline_url_base = mastodon_api + "/v1/timelines/public?limit=1"
last_toot_id = None

prev_account = ""

while True:
    if last_toot_id is not None:
        timeline_url = timeline_url_base + "&since_id=" + last_toot_id
    else:
        timeline_url = timeline_url_base
    timeline_rsp = urequests.get(url=timeline_url, headers=mastodon_headers)
    try:
        toots = timeline_rsp.json()
    except ValueError:
        print("JSON parse error, response: %s" % timeline_rsp.text)
        timeline_rsp.close()
        del timeline_rsp
        gc.collect()
        time.sleep(15)
        continue

    timeline_rsp.close()

    if len(toots) > 0:
        last_toot = toots[0]
        last_toot_id = last_toot["id"]

        if last_toot["reblog"]:
            last_toot = last_toot["reblog"]

        print("{display_name} @{acct}".format(**last_toot["account"]))
        # print(last_toot["content"])

        try:
            draw_toot(last_toot)
        except RuntimeError:
            print("Failed to draw toot: %s" % last_toot["content"])
            pass

        del last_toot

    del timeline_rsp, toots
    gc.collect()

    time.sleep(15)
