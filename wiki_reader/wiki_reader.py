"""
wiki_reader.py - Crude script to read a wikipedia article from an offset and an article ID

To find an article ID, use bzgrep on
enwiki-latest-pages-articles-multistream-index.txt.bz2
For example (in my shell *index* expands to enwiki-latest-pages-articles-multistream-index.txt.bz2):
bzgrep "Python (programming language)" *index*

Requires the bzip2 compressed dumps of Wikipedia. Download 
enwiki-latest-pages-articles-multistream-index.txt.bz2 and
enwiki-latest-pages-articles-multistream.xml.bz2
from https://dumps.wikimedia.org/enwiki/latest/

This script outputs MediaWiki. If you want to convert this to something more
familiar, then use pandoc:
python wiki_reader.py OFFSET ARTICLE_ID | pandoc -f mediawiki -t markdown | less
"""

import bz2
import re
import sys
import xml.etree.ElementTree as ET

# usage
if len(sys.argv) != 3:
    print(f'usage: {sys.argv[0]} offset articleID')
    exit(1)

# open file and seek
file = open('enwiki-latest-pages-articles-multistream.xml.bz2', 'rb')
file.seek(int(sys.argv[1]))

# decompress until we get something
dec = bz2.BZ2Decompressor()
txt = ''
while not dec.eof:
    txt += dec.decompress(file.read(4096)).decode()

# find the page with our ID
pages = re.findall(r'<page>.*?</page>', txt, flags=re.DOTALL)
found = None
for page in pages:
    tree = ET.fromstring(page)
    if sys.argv[2] == tree.findall('id')[0].text:
        found = tree

# print the result
if found is None:
    print(f'Article ID {sys.argv[2]} not found')
    exit(1)
print(found.findall('./revision/text')[0].text)

