from datetime import datetime
import os
import sys

str_head = '''<!DOCTYPE html><html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Cave Story MD Snapshots</title>
  <style>body { font: "Segoe UI", "Helvetica Neue", Arial, "Noto Sans", sans-serif; }
  @media (prefers-color-scheme: dark) {
    body { color: #EEE; background: #111; }
    a:link { color: #7BF; }
    a:visited { color: #9BF; }
  }</style>
</head><body>
<h2>Cave Story MD Automatic Snapshots</h2>
<h2 lang="ja">洞窟物語MD自動スナップショット</h2>
'''

str_row_start = '<p><strong>{date}</strong> (<a href="{file}">map</a>) -'
str_row_lang = ' <a href="{file}">{lang}</a> -'
str_row_end = '</p>\n'

str_foot = '<p>Last updated {date}</p></body></html>'


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
        snaps[i] += [[file.split('-')[1], file]]

    with open(os.path.join(sys.argv[1], 'index.html'), 'w') as f:
        f.write(str_head)

        for snap in snaps:
            f.write(str_row_start.format(date=snap[0][0], file=snap[0][1]))
            for param in sorted(snap[1:], key=lambda p: p[0]):
                f.write(str_row_lang.format(lang=param[0], file=param[1]))
            f.write(str_row_end)

        f.write(str_foot.format(date=datetime.now().strftime('%Y-%m-%d %H:%M:%S JST')))
