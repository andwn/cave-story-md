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

str_head = u'''<!DOCTYPE html>
<html lang="en"><head><meta charset="UTF-8">
<title>Cave Story MD Snapshots</title></head><body>
<link rel="stylesheet" href="https://tenshi.skychase.zone/style.css">
<style>table, th, td {
    border: 1px solid;
    border-collapse: collapse;
    padding: 4px;
    font-family: monospace, monospace;
}</style>
<h2>Cave Story MD Automatic Snapshots<br/>洞窟物語MD自動スナップショット</h2>'''

str_table_start = u'''<h4>{date} - <a href="{symbol}">Symbols</a></h4>
<table><tr><th>Language</th><th>ROM</th></tr>'''
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
            snaps += [[[datetime.strptime(newdate, '%Y%m%d%H%M').strftime('%Y-%m-%d %H:%M JST'),
                       'doukutsu-' + newdate + '.lst']]]
        snaps[i] += [[lang_map[file.split('-')[1]], file]]

    with open(os.path.join(sys.argv[1], 'index.html'), 'w') as f:
        f.write(str_head)

        for snap in snaps:
            f.write(str_table_start.format(date=snap[0][0], symbol=snap[0][1]))
            for param in sorted(snap[1:], key=lambda p: p[0]):
                f.write(str_table_row.format(lang=param[0], file=param[1]))
            f.write(str_table_end)

        f.write(str_foot.format(date=datetime.now().strftime('%Y-%m-%d %H:%M:%S JST')))
