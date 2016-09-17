import sys
html_template = file('charts-template.html', 'r').read()
sys.stdout.write(html_template.replace('__CHART_DATA_GOES_HERE__', sys.stdin.read()))
