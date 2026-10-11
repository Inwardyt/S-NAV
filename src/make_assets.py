from pathlib import Path
from PIL import Image,ImageFont,ImageDraw
root=Path(__file__).resolve().parents[2]
ref=next((root/'reference').glob('*/GFX'))
im=Image.open(root/'upload/Navigator.png').convert('RGB').quantize(colors=128)
pal=im.getpalette()
out=Path(__file__).with_name('assets.h')
with out.open('w') as f:
 f.write('/* Embedded supplied skin and source-game font; see CREDITS.txt. */\n')
 f.write('static const unsigned int skin_palette[128]={'+','.join(hex((pal[i*3]<<16)|(pal[i*3+1]<<8)|pal[i*3+2]) for i in range(128))+'};\n')
 f.write('static const unsigned char skin_pixels[434*509]={\n')
 data=list(im.getdata())
 for i in range(0,len(data),128):f.write(','.join(map(str,data[i:i+128]))+',\n')
 f.write('};\nstatic const unsigned short nav_glyphs[96][16]={\n')
 font=ImageFont.truetype(str(ref/'font/DS-DIGI/DS-Digital.TTF'),14)
 for c in range(32,128):
  glyph=Image.new('L',(10,16));d=ImageDraw.Draw(glyph);d.text((0,0),chr(c),font=font,fill=255)
  rows=[sum((1<<x) for x in range(10) if glyph.getpixel((x,y))>=90) for y in range(16)]
  f.write('{'+','.join(map(str,rows))+'},\n')
 f.write('};\n')
