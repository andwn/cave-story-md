# -*- coding: utf-8 -*-

from datetime import datetime
import os
import sys

lang_map = {
    'en': u'English',
    'es': u'Español',
    'pt': u'Português',
    'br': u'Português (Brasil)',
    'it': u'Italiano',
    'fr': u'Français',
    'de': u'Deutsch',
    'fi': u'Suomi',
    'ja': u'日本語',
    'zh': u'中文',
    'tw': u'中文(台灣)',
    'ko': u'한국어',
}

str_head = u'''<html><head><title>Cave Story MD Snapshots</title></head><body>
<h1>Cave Story MD Automatic Snapshots</h1>
<h2>洞窟物語MD自動スナップショット</h2>'''

str_table_start = u'<h3>{date}</h3>\n<table><tr><th>Language</th><th>ROM</th></tr>'
str_table_row = u'<tr><td>{lang}</td><td><a href="{file}">{file}</a></td></tr>'
str_table_end = u'</table>'

str_foot = u'''<p>Last updated: {date}</p>
</body></html>'''


if __name__ == '__main__':
    files = sorted([f for f in os.listdir(sys.argv[1]) if f.endswith('.bin')],
                   key=lambda n: int(n.split('-')[2].split('.')[0]), reverse=True)

    snaps = []
    i = -1
    date = ''
    for file in files:
        newdate = file.split('-')[2].split('.')[0]
        if date != newdate:
            i += 1
            date = newdate
            snaps += [[datetime.strptime(newdate, '%Y%m%d%H%M').strftime('%Y-%m-%d %H:%M JST')]]
        snaps[i] += [[lang_map[file.split('-')[1]], file]]

    print(str_head)

    for snap in snaps:
        print(str_table_start.format(date=snap[0]))
        for param in snap[1:]:
            print(str_table_row.format(lang=param[0], file=param[1]))
        print(str_table_end)

    print(str_foot.format(date=datetime.now().strftime('%Y-%m-%d %H:%M:%S JST')))
