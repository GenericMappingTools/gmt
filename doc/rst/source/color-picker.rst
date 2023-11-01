GMT Color Picker
################

**Hover to show the color name and RGB value, and click to copy to clipboard.**

.. raw:: html

    <style>
        .color-box {
            width: 30px; /* Increase the width to 30px */
            height: 30px; /* Increase the height to 30px */
            margin: 2px; /* Decreased horizontal spacing between columns */
            display: inline-block;
            cursor: pointer;
            transition: transform 0.2s, border 0.2s; /* Add smooth transition effects for transform and border */
            border: 1px solid rgba(0,0,0,.3); /* Initial transparent border */
        }
        .color-box:hover {
            transform: scale(1.5); /* Magnify the box on hover */
            border: 1px solid #000; /* Add a border on hover */
        }
        .color-row {
            text-align: center;
            margin-bottom: 0px; /* Decreased vertical spacing between rows */
        }
        #color-container {
            text-align: center;
        }
        #notification {
            display: none;
            text-align: center;
            background-color: #4CAF50;
            color: white;
            padding: 10px;
            position: fixed;
            bottom: 0;
            left: 50%;
            transform: translateX(-50%);
        }
    </style>

    <div id="color-container">
        <div class="color-row">
            <div class="color-box" title="Name: snow RGB: 255/250/250" style="background-color: rgb(255, 250, 250);" onclick="copyToClipboard('snow', '255/250/250', notification)"></div>
            <div class="color-box" title="Name: ghostwhite RGB: 248/248/255" style="background-color: rgb(248, 248, 255);" onclick="copyToClipboard('ghostwhite', '248/248/255', notification)"></div>
            <div class="color-box" title="Name: floralwhite RGB: 255/250/240" style="background-color: rgb(255, 250, 240);" onclick="copyToClipboard('floralwhite', '255/250/240', notification)"></div>
            <div class="color-box" title="Name: seashell RGB: 255/245/238" style="background-color: rgb(255, 245, 238);" onclick="copyToClipboard('seashell', '255/245/238', notification)"></div>
            <div class="color-box" title="Name: oldlace RGB: 253/245/230" style="background-color: rgb(253, 245, 230);" onclick="copyToClipboard('oldlace', '253/245/230', notification)"></div>
            <div class="color-box" title="Name: linen RGB: 250/240/230" style="background-color: rgb(250, 240, 230);" onclick="copyToClipboard('linen', '250/240/230', notification)"></div>
            <div class="color-box" title="Name: antiquewhite RGB: 250/235/215" style="background-color: rgb(250, 235, 215);" onclick="copyToClipboard('antiquewhite', '250/235/215', notification)"></div>
            <div class="color-box" title="Name: papayawhip RGB: 255/239/213" style="background-color: rgb(255, 239, 213);" onclick="copyToClipboard('papayawhip', '255/239/213', notification)"></div>
            <div class="color-box" title="Name: blanchedalmond RGB: 255/235/205" style="background-color: rgb(255, 235, 205);" onclick="copyToClipboard('blanchedalmond', '255/235/205', notification)"></div>
            <div class="color-box" title="Name: bisque RGB: 255/228/196" style="background-color: rgb(255, 228, 196);" onclick="copyToClipboard('bisque', '255/228/196', notification)"></div>
            <div class="color-box" title="Name: peachpuff RGB: 255/218/185" style="background-color: rgb(255, 218, 185);" onclick="copyToClipboard('peachpuff', '255/218/185', notification)"></div>
            <div class="color-box" title="Name: navajowhite RGB: 255/222/173" style="background-color: rgb(255, 222, 173);" onclick="copyToClipboard('navajowhite', '255/222/173', notification)"></div>
            <div class="color-box" title="Name: moccasin RGB: 255/228/181" style="background-color: rgb(255, 228, 181);" onclick="copyToClipboard('moccasin', '255/228/181', notification)"></div>
            <div class="color-box" title="Name: lemonchiffon RGB: 255/250/205" style="background-color: rgb(255, 250, 205);" onclick="copyToClipboard('lemonchiffon', '255/250/205', notification)"></div>
            <div class="color-box" title="Name: cornsilk RGB: 255/248/220" style="background-color: rgb(255, 248, 220);" onclick="copyToClipboard('cornsilk', '255/248/220', notification)"></div>
            <div class="color-box" title="Name: ivory RGB: 255/255/240" style="background-color: rgb(255, 255, 240);" onclick="copyToClipboard('ivory', '255/255/240', notification)"></div>
            <div class="color-box" title="Name: honeydew RGB: 240/255/240" style="background-color: rgb(240, 255, 240);" onclick="copyToClipboard('honeydew', '240/255/240', notification)"></div>
            <div class="color-box" title="Name: mintcream RGB: 245/255/250" style="background-color: rgb(245, 255, 250);" onclick="copyToClipboard('mintcream', '245/255/250', notification)"></div>
            <div class="color-box" title="Name: azure RGB: 240/255/255" style="background-color: rgb(240, 255, 255);" onclick="copyToClipboard('azure', '240/255/255', notification)"></div>
            <div class="color-box" title="Name: aliceblue RGB: 240/248/255" style="background-color: rgb(240, 248, 255);" onclick="copyToClipboard('aliceblue', '240/248/255', notification)"></div>
            <div class="color-box" title="Name: lavender RGB: 230/230/250" style="background-color: rgb(230, 230, 250);" onclick="copyToClipboard('lavender', '230/230/250', notification)"></div>
            <div class="color-box" title="Name: lavenderblush RGB: 255/240/245" style="background-color: rgb(255, 240, 245);" onclick="copyToClipboard('lavenderblush', '255/240/245', notification)"></div>
            <div class="color-box" title="Name: mistyrose RGB: 255/228/225" style="background-color: rgb(255, 228, 225);" onclick="copyToClipboard('mistyrose', '255/228/225', notification)"></div>
            <div class="color-box" title="Name: midnightblue RGB: 25/25/112" style="background-color: rgb(25, 25, 112);" onclick="copyToClipboard('midnightblue', '25/25/112', notification)"></div>
            <div class="color-box" title="Name: navy RGB: 0/0/128" style="background-color: rgb(0, 0, 128);" onclick="copyToClipboard('navy', '0/0/128', notification)"></div>
            <div class="color-box" title="Name: navyblue RGB: 0/0/128" style="background-color: rgb(0, 0, 128);" onclick="copyToClipboard('navyblue', '0/0/128', notification)"></div>
            <div class="color-box" title="Name: cornflowerblue RGB: 100/149/237" style="background-color: rgb(100, 149, 237);" onclick="copyToClipboard('cornflowerblue', '100/149/237', notification)"></div>
            <div class="color-box" title="Name: darkslateblue RGB: 72/61/139" style="background-color: rgb(72, 61, 139);" onclick="copyToClipboard('darkslateblue', '72/61/139', notification)"></div>
            <div class="color-box" title="Name: slateblue RGB: 106/90/205" style="background-color: rgb(106, 90, 205);" onclick="copyToClipboard('slateblue', '106/90/205', notification)"></div>
            <div class="color-box" title="Name: mediumslateblue RGB: 123/104/238" style="background-color: rgb(123, 104, 238);" onclick="copyToClipboard('mediumslateblue', '123/104/238', notification)"></div>
            <div class="color-box" title="Name: lightslateblue RGB: 132/112/255" style="background-color: rgb(132, 112, 255);" onclick="copyToClipboard('lightslateblue', '132/112/255', notification)"></div>
            <div class="color-box" title="Name: mediumblue RGB: 0/0/205" style="background-color: rgb(0, 0, 205);" onclick="copyToClipboard('mediumblue', '0/0/205', notification)"></div>
            <div class="color-box" title="Name: royalblue RGB: 65/105/225" style="background-color: rgb(65, 105, 225);" onclick="copyToClipboard('royalblue', '65/105/225', notification)"></div>
            <div class="color-box" title="Name: blue RGB: 0/0/255" style="background-color: rgb(0, 0, 255);" onclick="copyToClipboard('blue', '0/0/255', notification)"></div>
            <div class="color-box" title="Name: darkblue RGB: 0/0/139" style="background-color: rgb(0, 0, 139);" onclick="copyToClipboard('darkblue', '0/0/139', notification)"></div>
            <div class="color-box" title="Name: dodgerblue RGB: 30/144/255" style="background-color: rgb(30, 144, 255);" onclick="copyToClipboard('dodgerblue', '30/144/255', notification)"></div>
            <div class="color-box" title="Name: steelblue RGB: 70/130/180" style="background-color: rgb(70, 130, 180);" onclick="copyToClipboard('steelblue', '70/130/180', notification)"></div>
            <div class="color-box" title="Name: deepskyblue RGB: 0/191/255" style="background-color: rgb(0, 191, 255);" onclick="copyToClipboard('deepskyblue', '0/191/255', notification)"></div>
            <div class="color-box" title="Name: skyblue RGB: 135/206/235" style="background-color: rgb(135, 206, 235);" onclick="copyToClipboard('skyblue', '135/206/235', notification)"></div>
            <div class="color-box" title="Name: lightskyblue RGB: 135/206/250" style="background-color: rgb(135, 206, 250);" onclick="copyToClipboard('lightskyblue', '135/206/250', notification)"></div>
            <div class="color-box" title="Name: lightslategray RGB: 119/136/153" style="background-color: rgb(119, 136, 153);" onclick="copyToClipboard('lightslategray', '119/136/153', notification)"></div>
            <div class="color-box" title="Name: lightslategrey RGB: 119/136/153" style="background-color: rgb(119, 136, 153);" onclick="copyToClipboard('lightslategrey', '119/136/153', notification)"></div>
            <div class="color-box" title="Name: slategray RGB: 112/128/144" style="background-color: rgb(112, 128, 144);" onclick="copyToClipboard('slategray', '112/128/144', notification)"></div>
            <div class="color-box" title="Name: slategrey RGB: 112/128/144" style="background-color: rgb(112, 128, 144);" onclick="copyToClipboard('slategrey', '112/128/144', notification)"></div>
            <div class="color-box" title="Name: lightsteelblue RGB: 176/196/222" style="background-color: rgb(176, 196, 222);" onclick="copyToClipboard('lightsteelblue', '176/196/222', notification)"></div>
            <div class="color-box" title="Name: lightblue RGB: 173/216/230" style="background-color: rgb(173, 216, 230);" onclick="copyToClipboard('lightblue', '173/216/230', notification)"></div>
            <div class="color-box" title="Name: lightcyan RGB: 224/255/255" style="background-color: rgb(224, 255, 255);" onclick="copyToClipboard('lightcyan', '224/255/255', notification)"></div>
            <div class="color-box" title="Name: powderblue RGB: 176/224/230" style="background-color: rgb(176, 224, 230);" onclick="copyToClipboard('powderblue', '176/224/230', notification)"></div>
            <div class="color-box" title="Name: paleturquoise RGB: 175/238/238" style="background-color: rgb(175, 238, 238);" onclick="copyToClipboard('paleturquoise', '175/238/238', notification)"></div>
            <div class="color-box" title="Name: cadetblue RGB: 95/158/160" style="background-color: rgb(95, 158, 160);" onclick="copyToClipboard('cadetblue', '95/158/160', notification)"></div>
            <div class="color-box" title="Name: darkturquoise RGB: 0/206/209" style="background-color: rgb(0, 206, 209);" onclick="copyToClipboard('darkturquoise', '0/206/209', notification)"></div>
            <div class="color-box" title="Name: mediumturquoise RGB: 72/209/204" style="background-color: rgb(72, 209, 204);" onclick="copyToClipboard('mediumturquoise', '72/209/204', notification)"></div>
            <div class="color-box" title="Name: turquoise RGB: 64/224/208" style="background-color: rgb(64, 224, 208);" onclick="copyToClipboard('turquoise', '64/224/208', notification)"></div>
            <div class="color-box" title="Name: cyan RGB: 0/255/255" style="background-color: rgb(0, 255, 255);" onclick="copyToClipboard('cyan', '0/255/255', notification)"></div>
            <div class="color-box" title="Name: darkcyan RGB: 0/139/139" style="background-color: rgb(0, 139, 139);" onclick="copyToClipboard('darkcyan', '0/139/139', notification)"></div>
            <div class="color-box" title="Name: darkslategray RGB: 47/79/79" style="background-color: rgb(47, 79, 79);" onclick="copyToClipboard('darkslategray', '47/79/79', notification)"></div>
            <div class="color-box" title="Name: darkslategrey RGB: 47/79/79" style="background-color: rgb(47, 79, 79);" onclick="copyToClipboard('darkslategrey', '47/79/79', notification)"></div>
            <div class="color-box" title="Name: mediumaquamarine RGB: 102/205/170" style="background-color: rgb(102, 205, 170);" onclick="copyToClipboard('mediumaquamarine', '102/205/170', notification)"></div>
            <div class="color-box" title="Name: aquamarine RGB: 127/255/212" style="background-color: rgb(127, 255, 212);" onclick="copyToClipboard('aquamarine', '127/255/212', notification)"></div>
            <div class="color-box" title="Name: darkgreen RGB: 0/100/0" style="background-color: rgb(0, 100, 0);" onclick="copyToClipboard('darkgreen', '0/100/0', notification)"></div>
            <div class="color-box" title="Name: lightgreen RGB: 144/238/144" style="background-color: rgb(144, 238, 144);" onclick="copyToClipboard('lightgreen', '144/238/144', notification)"></div>
            <div class="color-box" title="Name: darkseagreen RGB: 143/188/143" style="background-color: rgb(143, 188, 143);" onclick="copyToClipboard('darkseagreen', '143/188/143', notification)"></div>
            <div class="color-box" title="Name: seagreen RGB: 46/139/87" style="background-color: rgb(46, 139, 87);" onclick="copyToClipboard('seagreen', '46/139/87', notification)"></div>
            <div class="color-box" title="Name: mediumseagreen RGB: 60/179/113" style="background-color: rgb(60, 179, 113);" onclick="copyToClipboard('mediumseagreen', '60/179/113', notification)"></div>
            <div class="color-box" title="Name: lightseagreen RGB: 32/178/170" style="background-color: rgb(32, 178, 170);" onclick="copyToClipboard('lightseagreen', '32/178/170', notification)"></div>
            <div class="color-box" title="Name: palegreen RGB: 152/251/152" style="background-color: rgb(152, 251, 152);" onclick="copyToClipboard('palegreen', '152/251/152', notification)"></div>
            <div class="color-box" title="Name: springgreen RGB: 0/255/127" style="background-color: rgb(0, 255, 127);" onclick="copyToClipboard('springgreen', '0/255/127', notification)"></div>
            <div class="color-box" title="Name: lawngreen RGB: 124/252/0" style="background-color: rgb(124, 252, 0);" onclick="copyToClipboard('lawngreen', '124/252/0', notification)"></div>
            <div class="color-box" title="Name: green RGB: 0/255/0" style="background-color: rgb(0, 255, 0);" onclick="copyToClipboard('green', '0/255/0', notification)"></div>
            <div class="color-box" title="Name: chartreuse RGB: 127/255/0" style="background-color: rgb(127, 255, 0);" onclick="copyToClipboard('chartreuse', '127/255/0', notification)"></div>
            <div class="color-box" title="Name: mediumspringgreen RGB: 0/250/154" style="background-color: rgb(0, 250, 154);" onclick="copyToClipboard('mediumspringgreen', '0/250/154', notification)"></div>
            <div class="color-box" title="Name: greenyellow RGB: 173/255/47" style="background-color: rgb(173, 255, 47);" onclick="copyToClipboard('greenyellow', '173/255/47', notification)"></div>
            <div class="color-box" title="Name: limegreen RGB: 50/205/50" style="background-color: rgb(50, 205, 50);" onclick="copyToClipboard('limegreen', '50/205/50', notification)"></div>
            <div class="color-box" title="Name: yellowgreen RGB: 154/205/50" style="background-color: rgb(154, 205, 50);" onclick="copyToClipboard('yellowgreen', '154/205/50', notification)"></div>
            <div class="color-box" title="Name: forestgreen RGB: 34/139/34" style="background-color: rgb(34, 139, 34);" onclick="copyToClipboard('forestgreen', '34/139/34', notification)"></div>
            <div class="color-box" title="Name: olivedrab RGB: 107/142/35" style="background-color: rgb(107, 142, 35);" onclick="copyToClipboard('olivedrab', '107/142/35', notification)"></div>
            <div class="color-box" title="Name: darkolivegreen RGB: 85/107/47" style="background-color: rgb(85, 107, 47);" onclick="copyToClipboard('darkolivegreen', '85/107/47', notification)"></div>
            <div class="color-box" title="Name: darkkhaki RGB: 189/183/107" style="background-color: rgb(189, 183, 107);" onclick="copyToClipboard('darkkhaki', '189/183/107', notification)"></div>
            <div class="color-box" title="Name: khaki RGB: 240/230/140" style="background-color: rgb(240, 230, 140);" onclick="copyToClipboard('khaki', '240/230/140', notification)"></div>
            <div class="color-box" title="Name: palegoldenrod RGB: 238/232/170" style="background-color: rgb(238, 232, 170);" onclick="copyToClipboard('palegoldenrod', '238/232/170', notification)"></div>
            <div class="color-box" title="Name: lightgoldenrod RGB: 238/221/130" style="background-color: rgb(238, 221, 130);" onclick="copyToClipboard('lightgoldenrod', '238/221/130', notification)"></div>
            <div class="color-box" title="Name: lightyellow RGB: 255/255/224" style="background-color: rgb(255, 255, 224);" onclick="copyToClipboard('lightyellow', '255/255/224', notification)"></div>
            <div class="color-box" title="Name: lightgoldenrodyellow RGB: 250/250/210" style="background-color: rgb(250, 250, 210);" onclick="copyToClipboard('lightgoldenrodyellow', '250/250/210', notification)"></div>
            <div class="color-box" title="Name: yellow RGB: 255/255/0" style="background-color: rgb(255, 255, 0);" onclick="copyToClipboard('yellow', '255/255/0', notification)"></div>
            <div class="color-box" title="Name: darkyellow RGB: 128/128/0" style="background-color: rgb(128, 128, 0);" onclick="copyToClipboard('darkyellow', '128/128/0', notification)"></div>
            <div class="color-box" title="Name: gold RGB: 255/215/0" style="background-color: rgb(255, 215, 0);" onclick="copyToClipboard('gold', '255/215/0', notification)"></div>
            <div class="color-box" title="Name: goldenrod RGB: 218/165/32" style="background-color: rgb(218, 165, 32);" onclick="copyToClipboard('goldenrod', '218/165/32', notification)"></div>
            <div class="color-box" title="Name: darkgoldenrod RGB: 184/134/11" style="background-color: rgb(184, 134, 11);" onclick="copyToClipboard('darkgoldenrod', '184/134/11', notification)"></div>
            <div class="color-box" title="Name: rosybrown RGB: 188/143/143" style="background-color: rgb(188, 143, 143);" onclick="copyToClipboard('rosybrown', '188/143/143', notification)"></div>
            <div class="color-box" title="Name: indianred RGB: 205/92/92" style="background-color: rgb(205, 92, 92);" onclick="copyToClipboard('indianred', '205/92/92', notification)"></div>
            <div class="color-box" title="Name: saddlebrown RGB: 139/69/19" style="background-color: rgb(139, 69, 19);" onclick="copyToClipboard('saddlebrown', '139/69/19', notification)"></div>
            <div class="color-box" title="Name: sienna RGB: 160/82/45" style="background-color: rgb(160, 82, 45);" onclick="copyToClipboard('sienna', '160/82/45', notification)"></div>
            <div class="color-box" title="Name: peru RGB: 205/133/63" style="background-color: rgb(205, 133, 63);" onclick="copyToClipboard('peru', '205/133/63', notification)"></div>
            <div class="color-box" title="Name: burlywood RGB: 222/184/135" style="background-color: rgb(222, 184, 135);" onclick="copyToClipboard('burlywood', '222/184/135', notification)"></div>
            <div class="color-box" title="Name: beige RGB: 245/245/220" style="background-color: rgb(245, 245, 220);" onclick="copyToClipboard('beige', '245/245/220', notification)"></div>
            <div class="color-box" title="Name: wheat RGB: 245/222/179" style="background-color: rgb(245, 222, 179);" onclick="copyToClipboard('wheat', '245/222/179', notification)"></div>
            <div class="color-box" title="Name: sandybrown RGB: 244/164/96" style="background-color: rgb(244, 164, 96);" onclick="copyToClipboard('sandybrown', '244/164/96', notification)"></div>
            <div class="color-box" title="Name: tan RGB: 210/180/140" style="background-color: rgb(210, 180, 140);" onclick="copyToClipboard('tan', '210/180/140', notification)"></div>
            <div class="color-box" title="Name: chocolate RGB: 210/105/30" style="background-color: rgb(210, 105, 30);" onclick="copyToClipboard('chocolate', '210/105/30', notification)"></div>
            <div class="color-box" title="Name: firebrick RGB: 178/34/34" style="background-color: rgb(178, 34, 34);" onclick="copyToClipboard('firebrick', '178/34/34', notification)"></div>
            <div class="color-box" title="Name: lightbrown RGB: 235/190/85" style="background-color: rgb(235, 190, 85);" onclick="copyToClipboard('lightbrown', '235/190/85', notification)"></div>
            <div class="color-box" title="Name: brown RGB: 165/42/42" style="background-color: rgb(165, 42, 42);" onclick="copyToClipboard('brown', '165/42/42', notification)"></div>
            <div class="color-box" title="Name: darkbrown RGB: 120/60/30" style="background-color: rgb(120, 60, 30);" onclick="copyToClipboard('darkbrown', '120/60/30', notification)"></div>
            <div class="color-box" title="Name: darksalmon RGB: 233/150/122" style="background-color: rgb(233, 150, 122);" onclick="copyToClipboard('darksalmon', '233/150/122', notification)"></div>
            <div class="color-box" title="Name: salmon RGB: 250/128/114" style="background-color: rgb(250, 128, 114);" onclick="copyToClipboard('salmon', '250/128/114', notification)"></div>
            <div class="color-box" title="Name: lightsalmon RGB: 255/160/122" style="background-color: rgb(255, 160, 122);" onclick="copyToClipboard('lightsalmon', '255/160/122', notification)"></div>
            <div class="color-box" title="Name: lightorange RGB: 255/192/128" style="background-color: rgb(255, 192, 128);" onclick="copyToClipboard('lightorange', '255/192/128', notification)"></div>
            <div class="color-box" title="Name: orange RGB: 255/165/0" style="background-color: rgb(255, 165, 0);" onclick="copyToClipboard('orange', '255/165/0', notification)"></div>
            <div class="color-box" title="Name: darkorange RGB: 255/140/0" style="background-color: rgb(255, 140, 0);" onclick="copyToClipboard('darkorange', '255/140/0', notification)"></div>
            <div class="color-box" title="Name: coral RGB: 255/127/80" style="background-color: rgb(255, 127, 80);" onclick="copyToClipboard('coral', '255/127/80', notification)"></div>
            <div class="color-box" title="Name: lightcoral RGB: 240/128/128" style="background-color: rgb(240, 128, 128);" onclick="copyToClipboard('lightcoral', '240/128/128', notification)"></div>
            <div class="color-box" title="Name: tomato RGB: 255/99/71" style="background-color: rgb(255, 99, 71);" onclick="copyToClipboard('tomato', '255/99/71', notification)"></div>
            <div class="color-box" title="Name: orangered RGB: 255/69/0" style="background-color: rgb(255, 69, 0);" onclick="copyToClipboard('orangered', '255/69/0', notification)"></div>
            <div class="color-box" title="Name: red RGB: 255/0/0" style="background-color: rgb(255, 0, 0);" onclick="copyToClipboard('red', '255/0/0', notification)"></div>
            <div class="color-box" title="Name: lightred RGB: 255/128/128" style="background-color: rgb(255, 128, 128);" onclick="copyToClipboard('lightred', '255/128/128', notification)"></div>
            <div class="color-box" title="Name: darkred RGB: 139/0/0" style="background-color: rgb(139, 0, 0);" onclick="copyToClipboard('darkred', '139/0/0', notification)"></div>
            <div class="color-box" title="Name: deeppink RGB: 255/20/147" style="background-color: rgb(255, 20, 147);" onclick="copyToClipboard('deeppink', '255/20/147', notification)"></div>
            <div class="color-box" title="Name: hotpink RGB: 255/105/180" style="background-color: rgb(255, 105, 180);" onclick="copyToClipboard('hotpink', '255/105/180', notification)"></div>
            <div class="color-box" title="Name: pink RGB: 255/192/203" style="background-color: rgb(255, 192, 203);" onclick="copyToClipboard('pink', '255/192/203', notification)"></div>
            <div class="color-box" title="Name: lightpink RGB: 255/182/193" style="background-color: rgb(255, 182, 193);" onclick="copyToClipboard('lightpink', '255/182/193', notification)"></div>
            <div class="color-box" title="Name: palevioletred RGB: 219/112/147" style="background-color: rgb(219, 112, 147);" onclick="copyToClipboard('palevioletred', '219/112/147', notification)"></div>
            <div class="color-box" title="Name: maroon RGB: 176/48/96" style="background-color: rgb(176, 48, 96);" onclick="copyToClipboard('maroon', '176/48/96', notification)"></div>
            <div class="color-box" title="Name: mediumvioletred RGB: 199/21/133" style="background-color: rgb(199, 21, 133);" onclick="copyToClipboard('mediumvioletred', '199/21/133', notification)"></div>
            <div class="color-box" title="Name: violetred RGB: 208/32/144" style="background-color: rgb(208, 32, 144);" onclick="copyToClipboard('violetred', '208/32/144', notification)"></div>
            <div class="color-box" title="Name: darkmagenta RGB: 139/0/139" style="background-color: rgb(139, 0, 139);" onclick="copyToClipboard('darkmagenta', '139/0/139', notification)"></div>
            <div class="color-box" title="Name: magenta RGB: 255/0/255" style="background-color: rgb(255, 0, 255);" onclick="copyToClipboard('magenta', '255/0/255', notification)"></div>
            <div class="color-box" title="Name: lightmagenta RGB: 255/128/255" style="background-color: rgb(255, 128, 255);" onclick="copyToClipboard('lightmagenta', '255/128/255', notification)"></div>
            <div class="color-box" title="Name: violet RGB: 238/130/238" style="background-color: rgb(238, 130, 238);" onclick="copyToClipboard('violet', '238/130/238', notification)"></div>
            <div class="color-box" title="Name: orchid RGB: 218/112/214" style="background-color: rgb(218, 112, 214);" onclick="copyToClipboard('orchid', '218/112/214', notification)"></div>
            <div class="color-box" title="Name: plum RGB: 221/160/221" style="background-color: rgb(221, 160, 221);" onclick="copyToClipboard('plum', '221/160/221', notification)"></div>
            <div class="color-box" title="Name: mediumorchid RGB: 186/85/211" style="background-color: rgb(186, 85, 211);" onclick="copyToClipboard('mediumorchid', '186/85/211', notification)"></div>
            <div class="color-box" title="Name: darkorchid RGB: 153/50/204" style="background-color: rgb(153, 50, 204);" onclick="copyToClipboard('darkorchid', '153/50/204', notification)"></div>
            <div class="color-box" title="Name: darkviolet RGB: 148/0/211" style="background-color: rgb(148, 0, 211);" onclick="copyToClipboard('darkviolet', '148/0/211', notification)"></div>
            <div class="color-box" title="Name: blueviolet RGB: 138/43/226" style="background-color: rgb(138, 43, 226);" onclick="copyToClipboard('blueviolet', '138/43/226', notification)"></div>
            <div class="color-box" title="Name: purple RGB: 160/32/240" style="background-color: rgb(160, 32, 240);" onclick="copyToClipboard('purple', '160/32/240', notification)"></div>
            <div class="color-box" title="Name: mediumpurple RGB: 147/112/219" style="background-color: rgb(147, 112, 219);" onclick="copyToClipboard('mediumpurple', '147/112/219', notification)"></div>
            <div class="color-box" title="Name: thistle RGB: 216/191/216" style="background-color: rgb(216, 191, 216);" onclick="copyToClipboard('thistle', '216/191/216', notification)"></div>
            <div class="color-box" title="Name: black RGB: 0/0/0" style="background-color: rgb(0, 0, 0);" onclick="copyToClipboard('black', '0/0/0', notification)"></div>
            <div class="color-box" title="Name: dimgray RGB: 105/105/105" style="background-color: rgb(105, 105, 105);" onclick="copyToClipboard('dimgray', '105/105/105', notification)"></div>
            <div class="color-box" title="Name: dimgrey RGB: 105/105/105" style="background-color: rgb(105, 105, 105);" onclick="copyToClipboard('dimgrey', '105/105/105', notification)"></div>
            <div class="color-box" title="Name: darkgray RGB: 169/169/169" style="background-color: rgb(169, 169, 169);" onclick="copyToClipboard('darkgray', '169/169/169', notification)"></div>
            <div class="color-box" title="Name: darkgrey RGB: 169/169/169" style="background-color: rgb(169, 169, 169);" onclick="copyToClipboard('darkgrey', '169/169/169', notification)"></div>
            <div class="color-box" title="Name: gray RGB: 190/190/190" style="background-color: rgb(190, 190, 190);" onclick="copyToClipboard('gray', '190/190/190', notification)"></div>
            <div class="color-box" title="Name: lightgrey RGB: 211/211/211" style="background-color: rgb(211, 211, 211);" onclick="copyToClipboard('lightgrey', '211/211/211', notification)"></div>
            <div class="color-box" title="Name: lightgray RGB: 211/211/211" style="background-color: rgb(211, 211, 211);" onclick="copyToClipboard('lightgray', '211/211/211', notification)"></div>
            <div class="color-box" title="Name: gainsboro RGB: 220/220/220" style="background-color: rgb(220, 220, 220);" onclick="copyToClipboard('gainsboro', '220/220/220', notification)"></div>
            <div class="color-box" title="Name: whitesmoke RGB: 245/245/245" style="background-color: rgb(245, 245, 245);" onclick="copyToClipboard('whitesmoke', '245/245/245', notification)"></div>
            <div class="color-box" title="Name: white RGB: 255/255/255" style="background-color: rgb(255, 255, 255);" onclick="copyToClipboard('white', '255/255/255', notification)"></div>
            <div class="color-box" title="Name: snow1 RGB: 255/250/250" style="background-color: rgb(255, 250, 250);" onclick="copyToClipboard('snow1', '255/250/250', notification)"></div>
            <div class="color-box" title="Name: snow2 RGB: 238/233/233" style="background-color: rgb(238, 233, 233);" onclick="copyToClipboard('snow2', '238/233/233', notification)"></div>
            <div class="color-box" title="Name: snow3 RGB: 205/201/201" style="background-color: rgb(205, 201, 201);" onclick="copyToClipboard('snow3', '205/201/201', notification)"></div>
            <div class="color-box" title="Name: snow4 RGB: 139/137/137" style="background-color: rgb(139, 137, 137);" onclick="copyToClipboard('snow4', '139/137/137', notification)"></div>
            <div class="color-box" title="Name: seashell1 RGB: 255/245/238" style="background-color: rgb(255, 245, 238);" onclick="copyToClipboard('seashell1', '255/245/238', notification)"></div>
            <div class="color-box" title="Name: seashell2 RGB: 238/229/222" style="background-color: rgb(238, 229, 222);" onclick="copyToClipboard('seashell2', '238/229/222', notification)"></div>
            <div class="color-box" title="Name: seashell3 RGB: 205/197/191" style="background-color: rgb(205, 197, 191);" onclick="copyToClipboard('seashell3', '205/197/191', notification)"></div>
            <div class="color-box" title="Name: seashell4 RGB: 139/134/130" style="background-color: rgb(139, 134, 130);" onclick="copyToClipboard('seashell4', '139/134/130', notification)"></div>
            <div class="color-box" title="Name: antiquewhite1 RGB: 255/239/219" style="background-color: rgb(255, 239, 219);" onclick="copyToClipboard('antiquewhite1', '255/239/219', notification)"></div>
            <div class="color-box" title="Name: antiquewhite2 RGB: 238/223/204" style="background-color: rgb(238, 223, 204);" onclick="copyToClipboard('antiquewhite2', '238/223/204', notification)"></div>
            <div class="color-box" title="Name: antiquewhite3 RGB: 205/192/176" style="background-color: rgb(205, 192, 176);" onclick="copyToClipboard('antiquewhite3', '205/192/176', notification)"></div>
            <div class="color-box" title="Name: antiquewhite4 RGB: 139/131/120" style="background-color: rgb(139, 131, 120);" onclick="copyToClipboard('antiquewhite4', '139/131/120', notification)"></div>
            <div class="color-box" title="Name: bisque1 RGB: 255/228/196" style="background-color: rgb(255, 228, 196);" onclick="copyToClipboard('bisque1', '255/228/196', notification)"></div>
            <div class="color-box" title="Name: bisque2 RGB: 238/213/183" style="background-color: rgb(238, 213, 183);" onclick="copyToClipboard('bisque2', '238/213/183', notification)"></div>
            <div class="color-box" title="Name: bisque3 RGB: 205/183/158" style="background-color: rgb(205, 183, 158);" onclick="copyToClipboard('bisque3', '205/183/158', notification)"></div>
            <div class="color-box" title="Name: bisque4 RGB: 139/125/107" style="background-color: rgb(139, 125, 107);" onclick="copyToClipboard('bisque4', '139/125/107', notification)"></div>
            <div class="color-box" title="Name: peachpuff1 RGB: 255/218/185" style="background-color: rgb(255, 218, 185);" onclick="copyToClipboard('peachpuff1', '255/218/185', notification)"></div>
            <div class="color-box" title="Name: peachpuff2 RGB: 238/203/173" style="background-color: rgb(238, 203, 173);" onclick="copyToClipboard('peachpuff2', '238/203/173', notification)"></div>
            <div class="color-box" title="Name: peachpuff3 RGB: 205/175/149" style="background-color: rgb(205, 175, 149);" onclick="copyToClipboard('peachpuff3', '205/175/149', notification)"></div>
            <div class="color-box" title="Name: peachpuff4 RGB: 139/119/101" style="background-color: rgb(139, 119, 101);" onclick="copyToClipboard('peachpuff4', '139/119/101', notification)"></div>
            <div class="color-box" title="Name: navajowhite1 RGB: 255/222/173" style="background-color: rgb(255, 222, 173);" onclick="copyToClipboard('navajowhite1', '255/222/173', notification)"></div>
            <div class="color-box" title="Name: navajowhite2 RGB: 238/207/161" style="background-color: rgb(238, 207, 161);" onclick="copyToClipboard('navajowhite2', '238/207/161', notification)"></div>
            <div class="color-box" title="Name: navajowhite3 RGB: 205/179/139" style="background-color: rgb(205, 179, 139);" onclick="copyToClipboard('navajowhite3', '205/179/139', notification)"></div>
            <div class="color-box" title="Name: navajowhite4 RGB: 139/121/94" style="background-color: rgb(139, 121, 94);" onclick="copyToClipboard('navajowhite4', '139/121/94', notification)"></div>
            <div class="color-box" title="Name: lemonchiffon1 RGB: 255/250/205" style="background-color: rgb(255, 250, 205);" onclick="copyToClipboard('lemonchiffon1', '255/250/205', notification)"></div>
            <div class="color-box" title="Name: lemonchiffon2 RGB: 238/233/191" style="background-color: rgb(238, 233, 191);" onclick="copyToClipboard('lemonchiffon2', '238/233/191', notification)"></div>
            <div class="color-box" title="Name: lemonchiffon3 RGB: 205/201/165" style="background-color: rgb(205, 201, 165);" onclick="copyToClipboard('lemonchiffon3', '205/201/165', notification)"></div>
            <div class="color-box" title="Name: lemonchiffon4 RGB: 139/137/112" style="background-color: rgb(139, 137, 112);" onclick="copyToClipboard('lemonchiffon4', '139/137/112', notification)"></div>
            <div class="color-box" title="Name: cornsilk1 RGB: 255/248/220" style="background-color: rgb(255, 248, 220);" onclick="copyToClipboard('cornsilk1', '255/248/220', notification)"></div>
            <div class="color-box" title="Name: cornsilk2 RGB: 238/232/205" style="background-color: rgb(238, 232, 205);" onclick="copyToClipboard('cornsilk2', '238/232/205', notification)"></div>
            <div class="color-box" title="Name: cornsilk3 RGB: 205/200/177" style="background-color: rgb(205, 200, 177);" onclick="copyToClipboard('cornsilk3', '205/200/177', notification)"></div>
            <div class="color-box" title="Name: cornsilk4 RGB: 139/136/120" style="background-color: rgb(139, 136, 120);" onclick="copyToClipboard('cornsilk4', '139/136/120', notification)"></div>
            <div class="color-box" title="Name: ivory1 RGB: 255/255/240" style="background-color: rgb(255, 255, 240);" onclick="copyToClipboard('ivory1', '255/255/240', notification)"></div>
            <div class="color-box" title="Name: ivory2 RGB: 238/238/224" style="background-color: rgb(238, 238, 224);" onclick="copyToClipboard('ivory2', '238/238/224', notification)"></div>
            <div class="color-box" title="Name: ivory3 RGB: 205/205/193" style="background-color: rgb(205, 205, 193);" onclick="copyToClipboard('ivory3', '205/205/193', notification)"></div>
            <div class="color-box" title="Name: ivory4 RGB: 139/139/131" style="background-color: rgb(139, 139, 131);" onclick="copyToClipboard('ivory4', '139/139/131', notification)"></div>
            <div class="color-box" title="Name: honeydew1 RGB: 240/255/240" style="background-color: rgb(240, 255, 240);" onclick="copyToClipboard('honeydew1', '240/255/240', notification)"></div>
            <div class="color-box" title="Name: honeydew2 RGB: 224/238/224" style="background-color: rgb(224, 238, 224);" onclick="copyToClipboard('honeydew2', '224/238/224', notification)"></div>
            <div class="color-box" title="Name: honeydew3 RGB: 193/205/193" style="background-color: rgb(193, 205, 193);" onclick="copyToClipboard('honeydew3', '193/205/193', notification)"></div>
            <div class="color-box" title="Name: honeydew4 RGB: 131/139/131" style="background-color: rgb(131, 139, 131);" onclick="copyToClipboard('honeydew4', '131/139/131', notification)"></div>
            <div class="color-box" title="Name: lavenderblush1 RGB: 255/240/245" style="background-color: rgb(255, 240, 245);" onclick="copyToClipboard('lavenderblush1', '255/240/245', notification)"></div>
            <div class="color-box" title="Name: lavenderblush2 RGB: 238/224/229" style="background-color: rgb(238, 224, 229);" onclick="copyToClipboard('lavenderblush2', '238/224/229', notification)"></div>
            <div class="color-box" title="Name: lavenderblush3 RGB: 205/193/197" style="background-color: rgb(205, 193, 197);" onclick="copyToClipboard('lavenderblush3', '205/193/197', notification)"></div>
            <div class="color-box" title="Name: lavenderblush4 RGB: 139/131/134" style="background-color: rgb(139, 131, 134);" onclick="copyToClipboard('lavenderblush4', '139/131/134', notification)"></div>
            <div class="color-box" title="Name: mistyrose1 RGB: 255/228/225" style="background-color: rgb(255, 228, 225);" onclick="copyToClipboard('mistyrose1', '255/228/225', notification)"></div>
            <div class="color-box" title="Name: mistyrose2 RGB: 238/213/210" style="background-color: rgb(238, 213, 210);" onclick="copyToClipboard('mistyrose2', '238/213/210', notification)"></div>
            <div class="color-box" title="Name: mistyrose3 RGB: 205/183/181" style="background-color: rgb(205, 183, 181);" onclick="copyToClipboard('mistyrose3', '205/183/181', notification)"></div>
            <div class="color-box" title="Name: mistyrose4 RGB: 139/125/123" style="background-color: rgb(139, 125, 123);" onclick="copyToClipboard('mistyrose4', '139/125/123', notification)"></div>
            <div class="color-box" title="Name: azure1 RGB: 240/255/255" style="background-color: rgb(240, 255, 255);" onclick="copyToClipboard('azure1', '240/255/255', notification)"></div>
            <div class="color-box" title="Name: azure2 RGB: 224/238/238" style="background-color: rgb(224, 238, 238);" onclick="copyToClipboard('azure2', '224/238/238', notification)"></div>
            <div class="color-box" title="Name: azure3 RGB: 193/205/205" style="background-color: rgb(193, 205, 205);" onclick="copyToClipboard('azure3', '193/205/205', notification)"></div>
            <div class="color-box" title="Name: azure4 RGB: 131/139/139" style="background-color: rgb(131, 139, 139);" onclick="copyToClipboard('azure4', '131/139/139', notification)"></div>
            <div class="color-box" title="Name: slateblue1 RGB: 131/111/255" style="background-color: rgb(131, 111, 255);" onclick="copyToClipboard('slateblue1', '131/111/255', notification)"></div>
            <div class="color-box" title="Name: slateblue2 RGB: 122/103/238" style="background-color: rgb(122, 103, 238);" onclick="copyToClipboard('slateblue2', '122/103/238', notification)"></div>
            <div class="color-box" title="Name: slateblue3 RGB: 105/89/205" style="background-color: rgb(105, 89, 205);" onclick="copyToClipboard('slateblue3', '105/89/205', notification)"></div>
            <div class="color-box" title="Name: slateblue4 RGB: 71/60/139" style="background-color: rgb(71, 60, 139);" onclick="copyToClipboard('slateblue4', '71/60/139', notification)"></div>
            <div class="color-box" title="Name: royalblue1 RGB: 72/118/255" style="background-color: rgb(72, 118, 255);" onclick="copyToClipboard('royalblue1', '72/118/255', notification)"></div>
            <div class="color-box" title="Name: royalblue2 RGB: 67/110/238" style="background-color: rgb(67, 110, 238);" onclick="copyToClipboard('royalblue2', '67/110/238', notification)"></div>
            <div class="color-box" title="Name: royalblue3 RGB: 58/95/205" style="background-color: rgb(58, 95, 205);" onclick="copyToClipboard('royalblue3', '58/95/205', notification)"></div>
            <div class="color-box" title="Name: royalblue4 RGB: 39/64/139" style="background-color: rgb(39, 64, 139);" onclick="copyToClipboard('royalblue4', '39/64/139', notification)"></div>
            <div class="color-box" title="Name: blue1 RGB: 0/0/255" style="background-color: rgb(0, 0, 255);" onclick="copyToClipboard('blue1', '0/0/255', notification)"></div>
            <div class="color-box" title="Name: blue2 RGB: 0/0/238" style="background-color: rgb(0, 0, 238);" onclick="copyToClipboard('blue2', '0/0/238', notification)"></div>
            <div class="color-box" title="Name: blue3 RGB: 0/0/205" style="background-color: rgb(0, 0, 205);" onclick="copyToClipboard('blue3', '0/0/205', notification)"></div>
            <div class="color-box" title="Name: blue4 RGB: 0/0/139" style="background-color: rgb(0, 0, 139);" onclick="copyToClipboard('blue4', '0/0/139', notification)"></div>
            <div class="color-box" title="Name: dodgerblue1 RGB: 30/144/255" style="background-color: rgb(30, 144, 255);" onclick="copyToClipboard('dodgerblue1', '30/144/255', notification)"></div>
            <div class="color-box" title="Name: dodgerblue2 RGB: 28/134/238" style="background-color: rgb(28, 134, 238);" onclick="copyToClipboard('dodgerblue2', '28/134/238', notification)"></div>
            <div class="color-box" title="Name: dodgerblue3 RGB: 24/116/205" style="background-color: rgb(24, 116, 205);" onclick="copyToClipboard('dodgerblue3', '24/116/205', notification)"></div>
            <div class="color-box" title="Name: dodgerblue4 RGB: 16/78/139" style="background-color: rgb(16, 78, 139);" onclick="copyToClipboard('dodgerblue4', '16/78/139', notification)"></div>
            <div class="color-box" title="Name: steelblue1 RGB: 99/184/255" style="background-color: rgb(99, 184, 255);" onclick="copyToClipboard('steelblue1', '99/184/255', notification)"></div>
            <div class="color-box" title="Name: steelblue2 RGB: 92/172/238" style="background-color: rgb(92, 172, 238);" onclick="copyToClipboard('steelblue2', '92/172/238', notification)"></div>
            <div class="color-box" title="Name: steelblue3 RGB: 79/148/205" style="background-color: rgb(79, 148, 205);" onclick="copyToClipboard('steelblue3', '79/148/205', notification)"></div>
            <div class="color-box" title="Name: steelblue4 RGB: 54/100/139" style="background-color: rgb(54, 100, 139);" onclick="copyToClipboard('steelblue4', '54/100/139', notification)"></div>
            <div class="color-box" title="Name: deepskyblue1 RGB: 0/191/255" style="background-color: rgb(0, 191, 255);" onclick="copyToClipboard('deepskyblue1', '0/191/255', notification)"></div>
            <div class="color-box" title="Name: deepskyblue2 RGB: 0/178/238" style="background-color: rgb(0, 178, 238);" onclick="copyToClipboard('deepskyblue2', '0/178/238', notification)"></div>
            <div class="color-box" title="Name: deepskyblue3 RGB: 0/154/205" style="background-color: rgb(0, 154, 205);" onclick="copyToClipboard('deepskyblue3', '0/154/205', notification)"></div>
            <div class="color-box" title="Name: deepskyblue4 RGB: 0/104/139" style="background-color: rgb(0, 104, 139);" onclick="copyToClipboard('deepskyblue4', '0/104/139', notification)"></div>
            <div class="color-box" title="Name: skyblue1 RGB: 135/206/255" style="background-color: rgb(135, 206, 255);" onclick="copyToClipboard('skyblue1', '135/206/255', notification)"></div>
            <div class="color-box" title="Name: skyblue2 RGB: 126/192/238" style="background-color: rgb(126, 192, 238);" onclick="copyToClipboard('skyblue2', '126/192/238', notification)"></div>
            <div class="color-box" title="Name: skyblue3 RGB: 108/166/205" style="background-color: rgb(108, 166, 205);" onclick="copyToClipboard('skyblue3', '108/166/205', notification)"></div>
            <div class="color-box" title="Name: skyblue4 RGB: 74/112/139" style="background-color: rgb(74, 112, 139);" onclick="copyToClipboard('skyblue4', '74/112/139', notification)"></div>
            <div class="color-box" title="Name: lightskyblue1 RGB: 176/226/255" style="background-color: rgb(176, 226, 255);" onclick="copyToClipboard('lightskyblue1', '176/226/255', notification)"></div>
            <div class="color-box" title="Name: lightskyblue2 RGB: 164/211/238" style="background-color: rgb(164, 211, 238);" onclick="copyToClipboard('lightskyblue2', '164/211/238', notification)"></div>
            <div class="color-box" title="Name: lightskyblue3 RGB: 141/182/205" style="background-color: rgb(141, 182, 205);" onclick="copyToClipboard('lightskyblue3', '141/182/205', notification)"></div>
            <div class="color-box" title="Name: lightskyblue4 RGB: 96/123/139" style="background-color: rgb(96, 123, 139);" onclick="copyToClipboard('lightskyblue4', '96/123/139', notification)"></div>
            <div class="color-box" title="Name: slategray1 RGB: 198/226/255" style="background-color: rgb(198, 226, 255);" onclick="copyToClipboard('slategray1', '198/226/255', notification)"></div>
            <div class="color-box" title="Name: slategray2 RGB: 185/211/238" style="background-color: rgb(185, 211, 238);" onclick="copyToClipboard('slategray2', '185/211/238', notification)"></div>
            <div class="color-box" title="Name: slategray3 RGB: 159/182/205" style="background-color: rgb(159, 182, 205);" onclick="copyToClipboard('slategray3', '159/182/205', notification)"></div>
            <div class="color-box" title="Name: slategray4 RGB: 108/123/139" style="background-color: rgb(108, 123, 139);" onclick="copyToClipboard('slategray4', '108/123/139', notification)"></div>
            <div class="color-box" title="Name: lightsteelblue1 RGB: 202/225/255" style="background-color: rgb(202, 225, 255);" onclick="copyToClipboard('lightsteelblue1', '202/225/255', notification)"></div>
            <div class="color-box" title="Name: lightsteelblue2 RGB: 188/210/238" style="background-color: rgb(188, 210, 238);" onclick="copyToClipboard('lightsteelblue2', '188/210/238', notification)"></div>
            <div class="color-box" title="Name: lightsteelblue3 RGB: 162/181/205" style="background-color: rgb(162, 181, 205);" onclick="copyToClipboard('lightsteelblue3', '162/181/205', notification)"></div>
            <div class="color-box" title="Name: lightsteelblue4 RGB: 110/123/139" style="background-color: rgb(110, 123, 139);" onclick="copyToClipboard('lightsteelblue4', '110/123/139', notification)"></div>
            <div class="color-box" title="Name: lightblue1 RGB: 191/239/255" style="background-color: rgb(191, 239, 255);" onclick="copyToClipboard('lightblue1', '191/239/255', notification)"></div>
            <div class="color-box" title="Name: lightblue2 RGB: 178/223/238" style="background-color: rgb(178, 223, 238);" onclick="copyToClipboard('lightblue2', '178/223/238', notification)"></div>
            <div class="color-box" title="Name: lightblue3 RGB: 154/192/205" style="background-color: rgb(154, 192, 205);" onclick="copyToClipboard('lightblue3', '154/192/205', notification)"></div>
            <div class="color-box" title="Name: lightblue4 RGB: 104/131/139" style="background-color: rgb(104, 131, 139);" onclick="copyToClipboard('lightblue4', '104/131/139', notification)"></div>
            <div class="color-box" title="Name: lightcyan1 RGB: 224/255/255" style="background-color: rgb(224, 255, 255);" onclick="copyToClipboard('lightcyan1', '224/255/255', notification)"></div>
            <div class="color-box" title="Name: lightcyan2 RGB: 209/238/238" style="background-color: rgb(209, 238, 238);" onclick="copyToClipboard('lightcyan2', '209/238/238', notification)"></div>
            <div class="color-box" title="Name: lightcyan3 RGB: 180/205/205" style="background-color: rgb(180, 205, 205);" onclick="copyToClipboard('lightcyan3', '180/205/205', notification)"></div>
            <div class="color-box" title="Name: lightcyan4 RGB: 122/139/139" style="background-color: rgb(122, 139, 139);" onclick="copyToClipboard('lightcyan4', '122/139/139', notification)"></div>
            <div class="color-box" title="Name: paleturquoise1 RGB: 187/255/255" style="background-color: rgb(187, 255, 255);" onclick="copyToClipboard('paleturquoise1', '187/255/255', notification)"></div>
            <div class="color-box" title="Name: paleturquoise2 RGB: 174/238/238" style="background-color: rgb(174, 238, 238);" onclick="copyToClipboard('paleturquoise2', '174/238/238', notification)"></div>
            <div class="color-box" title="Name: paleturquoise3 RGB: 150/205/205" style="background-color: rgb(150, 205, 205);" onclick="copyToClipboard('paleturquoise3', '150/205/205', notification)"></div>
            <div class="color-box" title="Name: paleturquoise4 RGB: 102/139/139" style="background-color: rgb(102, 139, 139);" onclick="copyToClipboard('paleturquoise4', '102/139/139', notification)"></div>
            <div class="color-box" title="Name: cadetblue1 RGB: 152/245/255" style="background-color: rgb(152, 245, 255);" onclick="copyToClipboard('cadetblue1', '152/245/255', notification)"></div>
            <div class="color-box" title="Name: cadetblue2 RGB: 142/229/238" style="background-color: rgb(142, 229, 238);" onclick="copyToClipboard('cadetblue2', '142/229/238', notification)"></div>
            <div class="color-box" title="Name: cadetblue3 RGB: 122/197/205" style="background-color: rgb(122, 197, 205);" onclick="copyToClipboard('cadetblue3', '122/197/205', notification)"></div>
            <div class="color-box" title="Name: cadetblue4 RGB: 83/134/139" style="background-color: rgb(83, 134, 139);" onclick="copyToClipboard('cadetblue4', '83/134/139', notification)"></div>
            <div class="color-box" title="Name: turquoise1 RGB: 0/245/255" style="background-color: rgb(0, 245, 255);" onclick="copyToClipboard('turquoise1', '0/245/255', notification)"></div>
            <div class="color-box" title="Name: turquoise2 RGB: 0/229/238" style="background-color: rgb(0, 229, 238);" onclick="copyToClipboard('turquoise2', '0/229/238', notification)"></div>
            <div class="color-box" title="Name: turquoise3 RGB: 0/197/205" style="background-color: rgb(0, 197, 205);" onclick="copyToClipboard('turquoise3', '0/197/205', notification)"></div>
            <div class="color-box" title="Name: turquoise4 RGB: 0/134/139" style="background-color: rgb(0, 134, 139);" onclick="copyToClipboard('turquoise4', '0/134/139', notification)"></div>
            <div class="color-box" title="Name: cyan1 RGB: 0/255/255" style="background-color: rgb(0, 255, 255);" onclick="copyToClipboard('cyan1', '0/255/255', notification)"></div>
            <div class="color-box" title="Name: cyan2 RGB: 0/238/238" style="background-color: rgb(0, 238, 238);" onclick="copyToClipboard('cyan2', '0/238/238', notification)"></div>
            <div class="color-box" title="Name: cyan3 RGB: 0/205/205" style="background-color: rgb(0, 205, 205);" onclick="copyToClipboard('cyan3', '0/205/205', notification)"></div>
            <div class="color-box" title="Name: cyan4 RGB: 0/139/139" style="background-color: rgb(0, 139, 139);" onclick="copyToClipboard('cyan4', '0/139/139', notification)"></div>
            <div class="color-box" title="Name: darkslategray1 RGB: 151/255/255" style="background-color: rgb(151, 255, 255);" onclick="copyToClipboard('darkslategray1', '151/255/255', notification)"></div>
            <div class="color-box" title="Name: darkslategray2 RGB: 141/238/238" style="background-color: rgb(141, 238, 238);" onclick="copyToClipboard('darkslategray2', '141/238/238', notification)"></div>
            <div class="color-box" title="Name: darkslategray3 RGB: 121/205/205" style="background-color: rgb(121, 205, 205);" onclick="copyToClipboard('darkslategray3', '121/205/205', notification)"></div>
            <div class="color-box" title="Name: darkslategray4 RGB: 82/139/139" style="background-color: rgb(82, 139, 139);" onclick="copyToClipboard('darkslategray4', '82/139/139', notification)"></div>
            <div class="color-box" title="Name: aquamarine1 RGB: 127/255/212" style="background-color: rgb(127, 255, 212);" onclick="copyToClipboard('aquamarine1', '127/255/212', notification)"></div>
            <div class="color-box" title="Name: aquamarine2 RGB: 118/238/198" style="background-color: rgb(118, 238, 198);" onclick="copyToClipboard('aquamarine2', '118/238/198', notification)"></div>
            <div class="color-box" title="Name: aquamarine3 RGB: 102/205/170" style="background-color: rgb(102, 205, 170);" onclick="copyToClipboard('aquamarine3', '102/205/170', notification)"></div>
            <div class="color-box" title="Name: aquamarine4 RGB: 69/139/116" style="background-color: rgb(69, 139, 116);" onclick="copyToClipboard('aquamarine4', '69/139/116', notification)"></div>
            <div class="color-box" title="Name: darkseagreen1 RGB: 193/255/193" style="background-color: rgb(193, 255, 193);" onclick="copyToClipboard('darkseagreen1', '193/255/193', notification)"></div>
            <div class="color-box" title="Name: darkseagreen2 RGB: 180/238/180" style="background-color: rgb(180, 238, 180);" onclick="copyToClipboard('darkseagreen2', '180/238/180', notification)"></div>
            <div class="color-box" title="Name: darkseagreen3 RGB: 155/205/155" style="background-color: rgb(155, 205, 155);" onclick="copyToClipboard('darkseagreen3', '155/205/155', notification)"></div>
            <div class="color-box" title="Name: darkseagreen4 RGB: 105/139/105" style="background-color: rgb(105, 139, 105);" onclick="copyToClipboard('darkseagreen4', '105/139/105', notification)"></div>
            <div class="color-box" title="Name: seagreen1 RGB: 84/255/159" style="background-color: rgb(84, 255, 159);" onclick="copyToClipboard('seagreen1', '84/255/159', notification)"></div>
            <div class="color-box" title="Name: seagreen2 RGB: 78/238/148" style="background-color: rgb(78, 238, 148);" onclick="copyToClipboard('seagreen2', '78/238/148', notification)"></div>
            <div class="color-box" title="Name: seagreen3 RGB: 67/205/128" style="background-color: rgb(67, 205, 128);" onclick="copyToClipboard('seagreen3', '67/205/128', notification)"></div>
            <div class="color-box" title="Name: seagreen4 RGB: 46/139/87" style="background-color: rgb(46, 139, 87);" onclick="copyToClipboard('seagreen4', '46/139/87', notification)"></div>
            <div class="color-box" title="Name: palegreen1 RGB: 154/255/154" style="background-color: rgb(154, 255, 154);" onclick="copyToClipboard('palegreen1', '154/255/154', notification)"></div>
            <div class="color-box" title="Name: palegreen2 RGB: 144/238/144" style="background-color: rgb(144, 238, 144);" onclick="copyToClipboard('palegreen2', '144/238/144', notification)"></div>
            <div class="color-box" title="Name: palegreen3 RGB: 124/205/124" style="background-color: rgb(124, 205, 124);" onclick="copyToClipboard('palegreen3', '124/205/124', notification)"></div>
            <div class="color-box" title="Name: palegreen4 RGB: 84/139/84" style="background-color: rgb(84, 139, 84);" onclick="copyToClipboard('palegreen4', '84/139/84', notification)"></div>
            <div class="color-box" title="Name: springgreen1 RGB: 0/255/127" style="background-color: rgb(0, 255, 127);" onclick="copyToClipboard('springgreen1', '0/255/127', notification)"></div>
            <div class="color-box" title="Name: springgreen2 RGB: 0/238/118" style="background-color: rgb(0, 238, 118);" onclick="copyToClipboard('springgreen2', '0/238/118', notification)"></div>
            <div class="color-box" title="Name: springgreen3 RGB: 0/205/102" style="background-color: rgb(0, 205, 102);" onclick="copyToClipboard('springgreen3', '0/205/102', notification)"></div>
            <div class="color-box" title="Name: springgreen4 RGB: 0/139/69" style="background-color: rgb(0, 139, 69);" onclick="copyToClipboard('springgreen4', '0/139/69', notification)"></div>
            <div class="color-box" title="Name: green1 RGB: 0/255/0" style="background-color: rgb(0, 255, 0);" onclick="copyToClipboard('green1', '0/255/0', notification)"></div>
            <div class="color-box" title="Name: green2 RGB: 0/238/0" style="background-color: rgb(0, 238, 0);" onclick="copyToClipboard('green2', '0/238/0', notification)"></div>
            <div class="color-box" title="Name: green3 RGB: 0/205/0" style="background-color: rgb(0, 205, 0);" onclick="copyToClipboard('green3', '0/205/0', notification)"></div>
            <div class="color-box" title="Name: green4 RGB: 0/139/0" style="background-color: rgb(0, 139, 0);" onclick="copyToClipboard('green4', '0/139/0', notification)"></div>
            <div class="color-box" title="Name: chartreuse1 RGB: 127/255/0" style="background-color: rgb(127, 255, 0);" onclick="copyToClipboard('chartreuse1', '127/255/0', notification)"></div>
            <div class="color-box" title="Name: chartreuse2 RGB: 118/238/0" style="background-color: rgb(118, 238, 0);" onclick="copyToClipboard('chartreuse2', '118/238/0', notification)"></div>
            <div class="color-box" title="Name: chartreuse3 RGB: 102/205/0" style="background-color: rgb(102, 205, 0);" onclick="copyToClipboard('chartreuse3', '102/205/0', notification)"></div>
            <div class="color-box" title="Name: chartreuse4 RGB: 69/139/0" style="background-color: rgb(69, 139, 0);" onclick="copyToClipboard('chartreuse4', '69/139/0', notification)"></div>
            <div class="color-box" title="Name: olivedrab1 RGB: 192/255/62" style="background-color: rgb(192, 255, 62);" onclick="copyToClipboard('olivedrab1', '192/255/62', notification)"></div>
            <div class="color-box" title="Name: olivedrab2 RGB: 179/238/58" style="background-color: rgb(179, 238, 58);" onclick="copyToClipboard('olivedrab2', '179/238/58', notification)"></div>
            <div class="color-box" title="Name: olivedrab3 RGB: 154/205/50" style="background-color: rgb(154, 205, 50);" onclick="copyToClipboard('olivedrab3', '154/205/50', notification)"></div>
            <div class="color-box" title="Name: olivedrab4 RGB: 105/139/34" style="background-color: rgb(105, 139, 34);" onclick="copyToClipboard('olivedrab4', '105/139/34', notification)"></div>
            <div class="color-box" title="Name: darkolivegreen1 RGB: 202/255/112" style="background-color: rgb(202, 255, 112);" onclick="copyToClipboard('darkolivegreen1', '202/255/112', notification)"></div>
            <div class="color-box" title="Name: darkolivegreen2 RGB: 188/238/104" style="background-color: rgb(188, 238, 104);" onclick="copyToClipboard('darkolivegreen2', '188/238/104', notification)"></div>
            <div class="color-box" title="Name: darkolivegreen3 RGB: 162/205/90" style="background-color: rgb(162, 205, 90);" onclick="copyToClipboard('darkolivegreen3', '162/205/90', notification)"></div>
            <div class="color-box" title="Name: darkolivegreen4 RGB: 110/139/61" style="background-color: rgb(110, 139, 61);" onclick="copyToClipboard('darkolivegreen4', '110/139/61', notification)"></div>
            <div class="color-box" title="Name: khaki1 RGB: 255/246/143" style="background-color: rgb(255, 246, 143);" onclick="copyToClipboard('khaki1', '255/246/143', notification)"></div>
            <div class="color-box" title="Name: khaki2 RGB: 238/230/133" style="background-color: rgb(238, 230, 133);" onclick="copyToClipboard('khaki2', '238/230/133', notification)"></div>
            <div class="color-box" title="Name: khaki3 RGB: 205/198/115" style="background-color: rgb(205, 198, 115);" onclick="copyToClipboard('khaki3', '205/198/115', notification)"></div>
            <div class="color-box" title="Name: khaki4 RGB: 139/134/78" style="background-color: rgb(139, 134, 78);" onclick="copyToClipboard('khaki4', '139/134/78', notification)"></div>
            <div class="color-box" title="Name: lightgoldenrod1 RGB: 255/236/139" style="background-color: rgb(255, 236, 139);" onclick="copyToClipboard('lightgoldenrod1', '255/236/139', notification)"></div>
            <div class="color-box" title="Name: lightgoldenrod2 RGB: 238/220/130" style="background-color: rgb(238, 220, 130);" onclick="copyToClipboard('lightgoldenrod2', '238/220/130', notification)"></div>
            <div class="color-box" title="Name: lightgoldenrod3 RGB: 205/190/112" style="background-color: rgb(205, 190, 112);" onclick="copyToClipboard('lightgoldenrod3', '205/190/112', notification)"></div>
            <div class="color-box" title="Name: lightgoldenrod4 RGB: 139/129/76" style="background-color: rgb(139, 129, 76);" onclick="copyToClipboard('lightgoldenrod4', '139/129/76', notification)"></div>
            <div class="color-box" title="Name: lightyellow1 RGB: 255/255/224" style="background-color: rgb(255, 255, 224);" onclick="copyToClipboard('lightyellow1', '255/255/224', notification)"></div>
            <div class="color-box" title="Name: lightyellow2 RGB: 238/238/209" style="background-color: rgb(238, 238, 209);" onclick="copyToClipboard('lightyellow2', '238/238/209', notification)"></div>
            <div class="color-box" title="Name: lightyellow3 RGB: 205/205/180" style="background-color: rgb(205, 205, 180);" onclick="copyToClipboard('lightyellow3', '205/205/180', notification)"></div>
            <div class="color-box" title="Name: lightyellow4 RGB: 139/139/122" style="background-color: rgb(139, 139, 122);" onclick="copyToClipboard('lightyellow4', '139/139/122', notification)"></div>
            <div class="color-box" title="Name: yellow1 RGB: 255/255/0" style="background-color: rgb(255, 255, 0);" onclick="copyToClipboard('yellow1', '255/255/0', notification)"></div>
            <div class="color-box" title="Name: yellow2 RGB: 238/238/0" style="background-color: rgb(238, 238, 0);" onclick="copyToClipboard('yellow2', '238/238/0', notification)"></div>
            <div class="color-box" title="Name: yellow3 RGB: 205/205/0" style="background-color: rgb(205, 205, 0);" onclick="copyToClipboard('yellow3', '205/205/0', notification)"></div>
            <div class="color-box" title="Name: yellow4 RGB: 139/139/0" style="background-color: rgb(139, 139, 0);" onclick="copyToClipboard('yellow4', '139/139/0', notification)"></div>
            <div class="color-box" title="Name: gold1 RGB: 255/215/0" style="background-color: rgb(255, 215, 0);" onclick="copyToClipboard('gold1', '255/215/0', notification)"></div>
            <div class="color-box" title="Name: gold2 RGB: 238/201/0" style="background-color: rgb(238, 201, 0);" onclick="copyToClipboard('gold2', '238/201/0', notification)"></div>
            <div class="color-box" title="Name: gold3 RGB: 205/173/0" style="background-color: rgb(205, 173, 0);" onclick="copyToClipboard('gold3', '205/173/0', notification)"></div>
            <div class="color-box" title="Name: gold4 RGB: 139/117/0" style="background-color: rgb(139, 117, 0);" onclick="copyToClipboard('gold4', '139/117/0', notification)"></div>
            <div class="color-box" title="Name: goldenrod1 RGB: 255/193/37" style="background-color: rgb(255, 193, 37);" onclick="copyToClipboard('goldenrod1', '255/193/37', notification)"></div>
            <div class="color-box" title="Name: goldenrod2 RGB: 238/180/34" style="background-color: rgb(238, 180, 34);" onclick="copyToClipboard('goldenrod2', '238/180/34', notification)"></div>
            <div class="color-box" title="Name: goldenrod3 RGB: 205/155/29" style="background-color: rgb(205, 155, 29);" onclick="copyToClipboard('goldenrod3', '205/155/29', notification)"></div>
            <div class="color-box" title="Name: goldenrod4 RGB: 139/105/20" style="background-color: rgb(139, 105, 20);" onclick="copyToClipboard('goldenrod4', '139/105/20', notification)"></div>
            <div class="color-box" title="Name: darkgoldenrod1 RGB: 255/185/15" style="background-color: rgb(255, 185, 15);" onclick="copyToClipboard('darkgoldenrod1', '255/185/15', notification)"></div>
            <div class="color-box" title="Name: darkgoldenrod2 RGB: 238/173/14" style="background-color: rgb(238, 173, 14);" onclick="copyToClipboard('darkgoldenrod2', '238/173/14', notification)"></div>
            <div class="color-box" title="Name: darkgoldenrod3 RGB: 205/149/12" style="background-color: rgb(205, 149, 12);" onclick="copyToClipboard('darkgoldenrod3', '205/149/12', notification)"></div>
            <div class="color-box" title="Name: darkgoldenrod4 RGB: 139/101/8" style="background-color: rgb(139, 101, 8);" onclick="copyToClipboard('darkgoldenrod4', '139/101/8', notification)"></div>
            <div class="color-box" title="Name: rosybrown1 RGB: 255/193/193" style="background-color: rgb(255, 193, 193);" onclick="copyToClipboard('rosybrown1', '255/193/193', notification)"></div>
            <div class="color-box" title="Name: rosybrown2 RGB: 238/180/180" style="background-color: rgb(238, 180, 180);" onclick="copyToClipboard('rosybrown2', '238/180/180', notification)"></div>
            <div class="color-box" title="Name: rosybrown3 RGB: 205/155/155" style="background-color: rgb(205, 155, 155);" onclick="copyToClipboard('rosybrown3', '205/155/155', notification)"></div>
            <div class="color-box" title="Name: rosybrown4 RGB: 139/105/105" style="background-color: rgb(139, 105, 105);" onclick="copyToClipboard('rosybrown4', '139/105/105', notification)"></div>
            <div class="color-box" title="Name: indianred1 RGB: 255/106/106" style="background-color: rgb(255, 106, 106);" onclick="copyToClipboard('indianred1', '255/106/106', notification)"></div>
            <div class="color-box" title="Name: indianred2 RGB: 238/99/99" style="background-color: rgb(238, 99, 99);" onclick="copyToClipboard('indianred2', '238/99/99', notification)"></div>
            <div class="color-box" title="Name: indianred3 RGB: 205/85/85" style="background-color: rgb(205, 85, 85);" onclick="copyToClipboard('indianred3', '205/85/85', notification)"></div>
            <div class="color-box" title="Name: indianred4 RGB: 139/58/58" style="background-color: rgb(139, 58, 58);" onclick="copyToClipboard('indianred4', '139/58/58', notification)"></div>
            <div class="color-box" title="Name: sienna1 RGB: 255/130/71" style="background-color: rgb(255, 130, 71);" onclick="copyToClipboard('sienna1', '255/130/71', notification)"></div>
            <div class="color-box" title="Name: sienna2 RGB: 238/121/66" style="background-color: rgb(238, 121, 66);" onclick="copyToClipboard('sienna2', '238/121/66', notification)"></div>
            <div class="color-box" title="Name: sienna3 RGB: 205/104/57" style="background-color: rgb(205, 104, 57);" onclick="copyToClipboard('sienna3', '205/104/57', notification)"></div>
            <div class="color-box" title="Name: sienna4 RGB: 139/71/38" style="background-color: rgb(139, 71, 38);" onclick="copyToClipboard('sienna4', '139/71/38', notification)"></div>
            <div class="color-box" title="Name: burlywood1 RGB: 255/211/155" style="background-color: rgb(255, 211, 155);" onclick="copyToClipboard('burlywood1', '255/211/155', notification)"></div>
            <div class="color-box" title="Name: burlywood2 RGB: 238/197/145" style="background-color: rgb(238, 197, 145);" onclick="copyToClipboard('burlywood2', '238/197/145', notification)"></div>
            <div class="color-box" title="Name: burlywood3 RGB: 205/170/125" style="background-color: rgb(205, 170, 125);" onclick="copyToClipboard('burlywood3', '205/170/125', notification)"></div>
            <div class="color-box" title="Name: burlywood4 RGB: 139/115/85" style="background-color: rgb(139, 115, 85);" onclick="copyToClipboard('burlywood4', '139/115/85', notification)"></div>
            <div class="color-box" title="Name: wheat1 RGB: 255/231/186" style="background-color: rgb(255, 231, 186);" onclick="copyToClipboard('wheat1', '255/231/186', notification)"></div>
            <div class="color-box" title="Name: wheat2 RGB: 238/216/174" style="background-color: rgb(238, 216, 174);" onclick="copyToClipboard('wheat2', '238/216/174', notification)"></div>
            <div class="color-box" title="Name: wheat3 RGB: 205/186/150" style="background-color: rgb(205, 186, 150);" onclick="copyToClipboard('wheat3', '205/186/150', notification)"></div>
            <div class="color-box" title="Name: wheat4 RGB: 139/126/102" style="background-color: rgb(139, 126, 102);" onclick="copyToClipboard('wheat4', '139/126/102', notification)"></div>
            <div class="color-box" title="Name: tan1 RGB: 255/165/79" style="background-color: rgb(255, 165, 79);" onclick="copyToClipboard('tan1', '255/165/79', notification)"></div>
            <div class="color-box" title="Name: tan2 RGB: 238/154/73" style="background-color: rgb(238, 154, 73);" onclick="copyToClipboard('tan2', '238/154/73', notification)"></div>
            <div class="color-box" title="Name: tan3 RGB: 205/133/63" style="background-color: rgb(205, 133, 63);" onclick="copyToClipboard('tan3', '205/133/63', notification)"></div>
            <div class="color-box" title="Name: tan4 RGB: 139/90/43" style="background-color: rgb(139, 90, 43);" onclick="copyToClipboard('tan4', '139/90/43', notification)"></div>
            <div class="color-box" title="Name: chocolate1 RGB: 255/127/36" style="background-color: rgb(255, 127, 36);" onclick="copyToClipboard('chocolate1', '255/127/36', notification)"></div>
            <div class="color-box" title="Name: chocolate2 RGB: 238/118/33" style="background-color: rgb(238, 118, 33);" onclick="copyToClipboard('chocolate2', '238/118/33', notification)"></div>
            <div class="color-box" title="Name: chocolate3 RGB: 205/102/29" style="background-color: rgb(205, 102, 29);" onclick="copyToClipboard('chocolate3', '205/102/29', notification)"></div>
            <div class="color-box" title="Name: chocolate4 RGB: 139/69/19" style="background-color: rgb(139, 69, 19);" onclick="copyToClipboard('chocolate4', '139/69/19', notification)"></div>
            <div class="color-box" title="Name: firebrick1 RGB: 255/48/48" style="background-color: rgb(255, 48, 48);" onclick="copyToClipboard('firebrick1', '255/48/48', notification)"></div>
            <div class="color-box" title="Name: firebrick2 RGB: 238/44/44" style="background-color: rgb(238, 44, 44);" onclick="copyToClipboard('firebrick2', '238/44/44', notification)"></div>
            <div class="color-box" title="Name: firebrick3 RGB: 205/38/38" style="background-color: rgb(205, 38, 38);" onclick="copyToClipboard('firebrick3', '205/38/38', notification)"></div>
            <div class="color-box" title="Name: firebrick4 RGB: 139/26/26" style="background-color: rgb(139, 26, 26);" onclick="copyToClipboard('firebrick4', '139/26/26', notification)"></div>
            <div class="color-box" title="Name: brown1 RGB: 255/64/64" style="background-color: rgb(255, 64, 64);" onclick="copyToClipboard('brown1', '255/64/64', notification)"></div>
            <div class="color-box" title="Name: brown2 RGB: 238/59/59" style="background-color: rgb(238, 59, 59);" onclick="copyToClipboard('brown2', '238/59/59', notification)"></div>
            <div class="color-box" title="Name: brown3 RGB: 205/51/51" style="background-color: rgb(205, 51, 51);" onclick="copyToClipboard('brown3', '205/51/51', notification)"></div>
            <div class="color-box" title="Name: brown4 RGB: 139/35/35" style="background-color: rgb(139, 35, 35);" onclick="copyToClipboard('brown4', '139/35/35', notification)"></div>
            <div class="color-box" title="Name: salmon1 RGB: 255/140/105" style="background-color: rgb(255, 140, 105);" onclick="copyToClipboard('salmon1', '255/140/105', notification)"></div>
            <div class="color-box" title="Name: salmon2 RGB: 238/130/98" style="background-color: rgb(238, 130, 98);" onclick="copyToClipboard('salmon2', '238/130/98', notification)"></div>
            <div class="color-box" title="Name: salmon3 RGB: 205/112/84" style="background-color: rgb(205, 112, 84);" onclick="copyToClipboard('salmon3', '205/112/84', notification)"></div>
            <div class="color-box" title="Name: salmon4 RGB: 139/76/57" style="background-color: rgb(139, 76, 57);" onclick="copyToClipboard('salmon4', '139/76/57', notification)"></div>
            <div class="color-box" title="Name: lightsalmon1 RGB: 255/160/122" style="background-color: rgb(255, 160, 122);" onclick="copyToClipboard('lightsalmon1', '255/160/122', notification)"></div>
            <div class="color-box" title="Name: lightsalmon2 RGB: 238/149/114" style="background-color: rgb(238, 149, 114);" onclick="copyToClipboard('lightsalmon2', '238/149/114', notification)"></div>
            <div class="color-box" title="Name: lightsalmon3 RGB: 205/129/98" style="background-color: rgb(205, 129, 98);" onclick="copyToClipboard('lightsalmon3', '205/129/98', notification)"></div>
            <div class="color-box" title="Name: lightsalmon4 RGB: 139/87/66" style="background-color: rgb(139, 87, 66);" onclick="copyToClipboard('lightsalmon4', '139/87/66', notification)"></div>
            <div class="color-box" title="Name: orange1 RGB: 255/165/0" style="background-color: rgb(255, 165, 0);" onclick="copyToClipboard('orange1', '255/165/0', notification)"></div>
            <div class="color-box" title="Name: orange2 RGB: 238/154/0" style="background-color: rgb(238, 154, 0);" onclick="copyToClipboard('orange2', '238/154/0', notification)"></div>
            <div class="color-box" title="Name: orange3 RGB: 205/133/0" style="background-color: rgb(205, 133, 0);" onclick="copyToClipboard('orange3', '205/133/0', notification)"></div>
            <div class="color-box" title="Name: orange4 RGB: 139/90/0" style="background-color: rgb(139, 90, 0);" onclick="copyToClipboard('orange4', '139/90/0', notification)"></div>
            <div class="color-box" title="Name: darkorange1 RGB: 255/127/0" style="background-color: rgb(255, 127, 0);" onclick="copyToClipboard('darkorange1', '255/127/0', notification)"></div>
            <div class="color-box" title="Name: darkorange2 RGB: 238/118/0" style="background-color: rgb(238, 118, 0);" onclick="copyToClipboard('darkorange2', '238/118/0', notification)"></div>
            <div class="color-box" title="Name: darkorange3 RGB: 205/102/0" style="background-color: rgb(205, 102, 0);" onclick="copyToClipboard('darkorange3', '205/102/0', notification)"></div>
            <div class="color-box" title="Name: darkorange4 RGB: 139/69/0" style="background-color: rgb(139, 69, 0);" onclick="copyToClipboard('darkorange4', '139/69/0', notification)"></div>
            <div class="color-box" title="Name: coral1 RGB: 255/114/86" style="background-color: rgb(255, 114, 86);" onclick="copyToClipboard('coral1', '255/114/86', notification)"></div>
            <div class="color-box" title="Name: coral2 RGB: 238/106/80" style="background-color: rgb(238, 106, 80);" onclick="copyToClipboard('coral2', '238/106/80', notification)"></div>
            <div class="color-box" title="Name: coral3 RGB: 205/91/69" style="background-color: rgb(205, 91, 69);" onclick="copyToClipboard('coral3', '205/91/69', notification)"></div>
            <div class="color-box" title="Name: coral4 RGB: 139/62/47" style="background-color: rgb(139, 62, 47);" onclick="copyToClipboard('coral4', '139/62/47', notification)"></div>
            <div class="color-box" title="Name: tomato1 RGB: 255/99/71" style="background-color: rgb(255, 99, 71);" onclick="copyToClipboard('tomato1', '255/99/71', notification)"></div>
            <div class="color-box" title="Name: tomato2 RGB: 238/92/66" style="background-color: rgb(238, 92, 66);" onclick="copyToClipboard('tomato2', '238/92/66', notification)"></div>
            <div class="color-box" title="Name: tomato3 RGB: 205/79/57" style="background-color: rgb(205, 79, 57);" onclick="copyToClipboard('tomato3', '205/79/57', notification)"></div>
            <div class="color-box" title="Name: tomato4 RGB: 139/54/38" style="background-color: rgb(139, 54, 38);" onclick="copyToClipboard('tomato4', '139/54/38', notification)"></div>
            <div class="color-box" title="Name: orangered1 RGB: 255/69/0" style="background-color: rgb(255, 69, 0);" onclick="copyToClipboard('orangered1', '255/69/0', notification)"></div>
            <div class="color-box" title="Name: orangered2 RGB: 238/64/0" style="background-color: rgb(238, 64, 0);" onclick="copyToClipboard('orangered2', '238/64/0', notification)"></div>
            <div class="color-box" title="Name: orangered3 RGB: 205/55/0" style="background-color: rgb(205, 55, 0);" onclick="copyToClipboard('orangered3', '205/55/0', notification)"></div>
            <div class="color-box" title="Name: orangered4 RGB: 139/37/0" style="background-color: rgb(139, 37, 0);" onclick="copyToClipboard('orangered4', '139/37/0', notification)"></div>
            <div class="color-box" title="Name: red1 RGB: 255/0/0" style="background-color: rgb(255, 0, 0);" onclick="copyToClipboard('red1', '255/0/0', notification)"></div>
            <div class="color-box" title="Name: red2 RGB: 238/0/0" style="background-color: rgb(238, 0, 0);" onclick="copyToClipboard('red2', '238/0/0', notification)"></div>
            <div class="color-box" title="Name: red3 RGB: 205/0/0" style="background-color: rgb(205, 0, 0);" onclick="copyToClipboard('red3', '205/0/0', notification)"></div>
            <div class="color-box" title="Name: red4 RGB: 139/0/0" style="background-color: rgb(139, 0, 0);" onclick="copyToClipboard('red4', '139/0/0', notification)"></div>
            <div class="color-box" title="Name: deeppink1 RGB: 255/20/147" style="background-color: rgb(255, 20, 147);" onclick="copyToClipboard('deeppink1', '255/20/147', notification)"></div>
            <div class="color-box" title="Name: deeppink2 RGB: 238/18/137" style="background-color: rgb(238, 18, 137);" onclick="copyToClipboard('deeppink2', '238/18/137', notification)"></div>
            <div class="color-box" title="Name: deeppink3 RGB: 205/16/118" style="background-color: rgb(205, 16, 118);" onclick="copyToClipboard('deeppink3', '205/16/118', notification)"></div>
            <div class="color-box" title="Name: deeppink4 RGB: 139/10/80" style="background-color: rgb(139, 10, 80);" onclick="copyToClipboard('deeppink4', '139/10/80', notification)"></div>
            <div class="color-box" title="Name: hotpink1 RGB: 255/110/180" style="background-color: rgb(255, 110, 180);" onclick="copyToClipboard('hotpink1', '255/110/180', notification)"></div>
            <div class="color-box" title="Name: hotpink2 RGB: 238/106/167" style="background-color: rgb(238, 106, 167);" onclick="copyToClipboard('hotpink2', '238/106/167', notification)"></div>
            <div class="color-box" title="Name: hotpink3 RGB: 205/96/144" style="background-color: rgb(205, 96, 144);" onclick="copyToClipboard('hotpink3', '205/96/144', notification)"></div>
            <div class="color-box" title="Name: hotpink4 RGB: 139/58/98" style="background-color: rgb(139, 58, 98);" onclick="copyToClipboard('hotpink4', '139/58/98', notification)"></div>
            <div class="color-box" title="Name: pink1 RGB: 255/181/197" style="background-color: rgb(255, 181, 197);" onclick="copyToClipboard('pink1', '255/181/197', notification)"></div>
            <div class="color-box" title="Name: pink2 RGB: 238/169/184" style="background-color: rgb(238, 169, 184);" onclick="copyToClipboard('pink2', '238/169/184', notification)"></div>
            <div class="color-box" title="Name: pink3 RGB: 205/145/158" style="background-color: rgb(205, 145, 158);" onclick="copyToClipboard('pink3', '205/145/158', notification)"></div>
            <div class="color-box" title="Name: pink4 RGB: 139/99/108" style="background-color: rgb(139, 99, 108);" onclick="copyToClipboard('pink4', '139/99/108', notification)"></div>
            <div class="color-box" title="Name: lightpink1 RGB: 255/174/185" style="background-color: rgb(255, 174, 185);" onclick="copyToClipboard('lightpink1', '255/174/185', notification)"></div>
            <div class="color-box" title="Name: lightpink2 RGB: 238/162/173" style="background-color: rgb(238, 162, 173);" onclick="copyToClipboard('lightpink2', '238/162/173', notification)"></div>
            <div class="color-box" title="Name: lightpink3 RGB: 205/140/149" style="background-color: rgb(205, 140, 149);" onclick="copyToClipboard('lightpink3', '205/140/149', notification)"></div>
            <div class="color-box" title="Name: lightpink4 RGB: 139/95/101" style="background-color: rgb(139, 95, 101);" onclick="copyToClipboard('lightpink4', '139/95/101', notification)"></div>
            <div class="color-box" title="Name: palevioletred1 RGB: 255/130/171" style="background-color: rgb(255, 130, 171);" onclick="copyToClipboard('palevioletred1', '255/130/171', notification)"></div>
            <div class="color-box" title="Name: palevioletred2 RGB: 238/121/159" style="background-color: rgb(238, 121, 159);" onclick="copyToClipboard('palevioletred2', '238/121/159', notification)"></div>
            <div class="color-box" title="Name: palevioletred3 RGB: 205/104/137" style="background-color: rgb(205, 104, 137);" onclick="copyToClipboard('palevioletred3', '205/104/137', notification)"></div>
            <div class="color-box" title="Name: palevioletred4 RGB: 139/71/93" style="background-color: rgb(139, 71, 93);" onclick="copyToClipboard('palevioletred4', '139/71/93', notification)"></div>
            <div class="color-box" title="Name: maroon1 RGB: 255/52/179" style="background-color: rgb(255, 52, 179);" onclick="copyToClipboard('maroon1', '255/52/179', notification)"></div>
            <div class="color-box" title="Name: maroon2 RGB: 238/48/167" style="background-color: rgb(238, 48, 167);" onclick="copyToClipboard('maroon2', '238/48/167', notification)"></div>
            <div class="color-box" title="Name: maroon3 RGB: 205/41/144" style="background-color: rgb(205, 41, 144);" onclick="copyToClipboard('maroon3', '205/41/144', notification)"></div>
            <div class="color-box" title="Name: maroon4 RGB: 139/28/98" style="background-color: rgb(139, 28, 98);" onclick="copyToClipboard('maroon4', '139/28/98', notification)"></div>
            <div class="color-box" title="Name: violetred1 RGB: 255/62/150" style="background-color: rgb(255, 62, 150);" onclick="copyToClipboard('violetred1', '255/62/150', notification)"></div>
            <div class="color-box" title="Name: violetred2 RGB: 238/58/140" style="background-color: rgb(238, 58, 140);" onclick="copyToClipboard('violetred2', '238/58/140', notification)"></div>
            <div class="color-box" title="Name: violetred3 RGB: 205/50/120" style="background-color: rgb(205, 50, 120);" onclick="copyToClipboard('violetred3', '205/50/120', notification)"></div>
            <div class="color-box" title="Name: violetred4 RGB: 139/34/82" style="background-color: rgb(139, 34, 82);" onclick="copyToClipboard('violetred4', '139/34/82', notification)"></div>
            <div class="color-box" title="Name: magenta1 RGB: 255/0/255" style="background-color: rgb(255, 0, 255);" onclick="copyToClipboard('magenta1', '255/0/255', notification)"></div>
            <div class="color-box" title="Name: magenta2 RGB: 238/0/238" style="background-color: rgb(238, 0, 238);" onclick="copyToClipboard('magenta2', '238/0/238', notification)"></div>
            <div class="color-box" title="Name: magenta3 RGB: 205/0/205" style="background-color: rgb(205, 0, 205);" onclick="copyToClipboard('magenta3', '205/0/205', notification)"></div>
            <div class="color-box" title="Name: magenta4 RGB: 139/0/139" style="background-color: rgb(139, 0, 139);" onclick="copyToClipboard('magenta4', '139/0/139', notification)"></div>
            <div class="color-box" title="Name: orchid1 RGB: 255/131/250" style="background-color: rgb(255, 131, 250);" onclick="copyToClipboard('orchid1', '255/131/250', notification)"></div>
            <div class="color-box" title="Name: orchid2 RGB: 238/122/233" style="background-color: rgb(238, 122, 233);" onclick="copyToClipboard('orchid2', '238/122/233', notification)"></div>
            <div class="color-box" title="Name: orchid3 RGB: 205/105/201" style="background-color: rgb(205, 105, 201);" onclick="copyToClipboard('orchid3', '205/105/201', notification)"></div>
            <div class="color-box" title="Name: orchid4 RGB: 139/71/137" style="background-color: rgb(139, 71, 137);" onclick="copyToClipboard('orchid4', '139/71/137', notification)"></div>
            <div class="color-box" title="Name: plum1 RGB: 255/187/255" style="background-color: rgb(255, 187, 255);" onclick="copyToClipboard('plum1', '255/187/255', notification)"></div>
            <div class="color-box" title="Name: plum2 RGB: 238/174/238" style="background-color: rgb(238, 174, 238);" onclick="copyToClipboard('plum2', '238/174/238', notification)"></div>
            <div class="color-box" title="Name: plum3 RGB: 205/150/205" style="background-color: rgb(205, 150, 205);" onclick="copyToClipboard('plum3', '205/150/205', notification)"></div>
            <div class="color-box" title="Name: plum4 RGB: 139/102/139" style="background-color: rgb(139, 102, 139);" onclick="copyToClipboard('plum4', '139/102/139', notification)"></div>
            <div class="color-box" title="Name: mediumorchid1 RGB: 224/102/255" style="background-color: rgb(224, 102, 255);" onclick="copyToClipboard('mediumorchid1', '224/102/255', notification)"></div>
            <div class="color-box" title="Name: mediumorchid2 RGB: 209/95/238" style="background-color: rgb(209, 95, 238);" onclick="copyToClipboard('mediumorchid2', '209/95/238', notification)"></div>
            <div class="color-box" title="Name: mediumorchid3 RGB: 180/82/205" style="background-color: rgb(180, 82, 205);" onclick="copyToClipboard('mediumorchid3', '180/82/205', notification)"></div>
            <div class="color-box" title="Name: mediumorchid4 RGB: 122/55/139" style="background-color: rgb(122, 55, 139);" onclick="copyToClipboard('mediumorchid4', '122/55/139', notification)"></div>
            <div class="color-box" title="Name: darkorchid1 RGB: 191/62/255" style="background-color: rgb(191, 62, 255);" onclick="copyToClipboard('darkorchid1', '191/62/255', notification)"></div>
            <div class="color-box" title="Name: darkorchid2 RGB: 178/58/238" style="background-color: rgb(178, 58, 238);" onclick="copyToClipboard('darkorchid2', '178/58/238', notification)"></div>
            <div class="color-box" title="Name: darkorchid3 RGB: 154/50/205" style="background-color: rgb(154, 50, 205);" onclick="copyToClipboard('darkorchid3', '154/50/205', notification)"></div>
            <div class="color-box" title="Name: darkorchid4 RGB: 104/34/139" style="background-color: rgb(104, 34, 139);" onclick="copyToClipboard('darkorchid4', '104/34/139', notification)"></div>
            <div class="color-box" title="Name: purple1 RGB: 155/48/255" style="background-color: rgb(155, 48, 255);" onclick="copyToClipboard('purple1', '155/48/255', notification)"></div>
            <div class="color-box" title="Name: purple2 RGB: 145/44/238" style="background-color: rgb(145, 44, 238);" onclick="copyToClipboard('purple2', '145/44/238', notification)"></div>
            <div class="color-box" title="Name: purple3 RGB: 125/38/205" style="background-color: rgb(125, 38, 205);" onclick="copyToClipboard('purple3', '125/38/205', notification)"></div>
            <div class="color-box" title="Name: purple4 RGB: 85/26/139" style="background-color: rgb(85, 26, 139);" onclick="copyToClipboard('purple4', '85/26/139', notification)"></div>
            <div class="color-box" title="Name: mediumpurple1 RGB: 171/130/255" style="background-color: rgb(171, 130, 255);" onclick="copyToClipboard('mediumpurple1', '171/130/255', notification)"></div>
            <div class="color-box" title="Name: mediumpurple2 RGB: 159/121/238" style="background-color: rgb(159, 121, 238);" onclick="copyToClipboard('mediumpurple2', '159/121/238', notification)"></div>
            <div class="color-box" title="Name: mediumpurple3 RGB: 137/104/205" style="background-color: rgb(137, 104, 205);" onclick="copyToClipboard('mediumpurple3', '137/104/205', notification)"></div>
            <div class="color-box" title="Name: mediumpurple4 RGB: 93/71/139" style="background-color: rgb(93, 71, 139);" onclick="copyToClipboard('mediumpurple4', '93/71/139', notification)"></div>
            <div class="color-box" title="Name: thistle1 RGB: 255/225/255" style="background-color: rgb(255, 225, 255);" onclick="copyToClipboard('thistle1', '255/225/255', notification)"></div>
            <div class="color-box" title="Name: thistle2 RGB: 238/210/238" style="background-color: rgb(238, 210, 238);" onclick="copyToClipboard('thistle2', '238/210/238', notification)"></div>
            <div class="color-box" title="Name: thistle3 RGB: 205/181/205" style="background-color: rgb(205, 181, 205);" onclick="copyToClipboard('thistle3', '205/181/205', notification)"></div>
            <div class="color-box" title="Name: thistle4 RGB: 139/123/139" style="background-color: rgb(139, 123, 139);" onclick="copyToClipboard('thistle4', '139/123/139', notification)"></div>
            <div class="color-box" title="Name: gray0 RGB: 0/0/0" style="background-color: rgb(0, 0, 0);" onclick="copyToClipboard('gray0', '0/0/0', notification)"></div>
            <div class="color-box" title="Name: gray1 RGB: 3/3/3" style="background-color: rgb(3, 3, 3);" onclick="copyToClipboard('gray1', '3/3/3', notification)"></div>
            <div class="color-box" title="Name: gray2 RGB: 5/5/5" style="background-color: rgb(5, 5, 5);" onclick="copyToClipboard('gray2', '5/5/5', notification)"></div>
            <div class="color-box" title="Name: gray3 RGB: 8/8/8" style="background-color: rgb(8, 8, 8);" onclick="copyToClipboard('gray3', '8/8/8', notification)"></div>
            <div class="color-box" title="Name: gray4 RGB: 10/10/10" style="background-color: rgb(10, 10, 10);" onclick="copyToClipboard('gray4', '10/10/10', notification)"></div>
            <div class="color-box" title="Name: gray5 RGB: 13/13/13" style="background-color: rgb(13, 13, 13);" onclick="copyToClipboard('gray5', '13/13/13', notification)"></div>
            <div class="color-box" title="Name: gray6 RGB: 15/15/15" style="background-color: rgb(15, 15, 15);" onclick="copyToClipboard('gray6', '15/15/15', notification)"></div>
            <div class="color-box" title="Name: gray7 RGB: 18/18/18" style="background-color: rgb(18, 18, 18);" onclick="copyToClipboard('gray7', '18/18/18', notification)"></div>
            <div class="color-box" title="Name: gray8 RGB: 20/20/20" style="background-color: rgb(20, 20, 20);" onclick="copyToClipboard('gray8', '20/20/20', notification)"></div>
            <div class="color-box" title="Name: gray9 RGB: 23/23/23" style="background-color: rgb(23, 23, 23);" onclick="copyToClipboard('gray9', '23/23/23', notification)"></div>
            <div class="color-box" title="Name: gray10 RGB: 26/26/26" style="background-color: rgb(26, 26, 26);" onclick="copyToClipboard('gray10', '26/26/26', notification)"></div>
            <div class="color-box" title="Name: gray11 RGB: 28/28/28" style="background-color: rgb(28, 28, 28);" onclick="copyToClipboard('gray11', '28/28/28', notification)"></div>
            <div class="color-box" title="Name: gray12 RGB: 31/31/31" style="background-color: rgb(31, 31, 31);" onclick="copyToClipboard('gray12', '31/31/31', notification)"></div>
            <div class="color-box" title="Name: gray13 RGB: 33/33/33" style="background-color: rgb(33, 33, 33);" onclick="copyToClipboard('gray13', '33/33/33', notification)"></div>
            <div class="color-box" title="Name: gray14 RGB: 36/36/36" style="background-color: rgb(36, 36, 36);" onclick="copyToClipboard('gray14', '36/36/36', notification)"></div>
            <div class="color-box" title="Name: gray15 RGB: 38/38/38" style="background-color: rgb(38, 38, 38);" onclick="copyToClipboard('gray15', '38/38/38', notification)"></div>
            <div class="color-box" title="Name: gray16 RGB: 41/41/41" style="background-color: rgb(41, 41, 41);" onclick="copyToClipboard('gray16', '41/41/41', notification)"></div>
            <div class="color-box" title="Name: gray17 RGB: 43/43/43" style="background-color: rgb(43, 43, 43);" onclick="copyToClipboard('gray17', '43/43/43', notification)"></div>
            <div class="color-box" title="Name: gray18 RGB: 46/46/46" style="background-color: rgb(46, 46, 46);" onclick="copyToClipboard('gray18', '46/46/46', notification)"></div>
            <div class="color-box" title="Name: gray19 RGB: 48/48/48" style="background-color: rgb(48, 48, 48);" onclick="copyToClipboard('gray19', '48/48/48', notification)"></div>
            <div class="color-box" title="Name: gray20 RGB: 51/51/51" style="background-color: rgb(51, 51, 51);" onclick="copyToClipboard('gray20', '51/51/51', notification)"></div>
            <div class="color-box" title="Name: gray21 RGB: 54/54/54" style="background-color: rgb(54, 54, 54);" onclick="copyToClipboard('gray21', '54/54/54', notification)"></div>
            <div class="color-box" title="Name: gray22 RGB: 56/56/56" style="background-color: rgb(56, 56, 56);" onclick="copyToClipboard('gray22', '56/56/56', notification)"></div>
            <div class="color-box" title="Name: gray23 RGB: 59/59/59" style="background-color: rgb(59, 59, 59);" onclick="copyToClipboard('gray23', '59/59/59', notification)"></div>
            <div class="color-box" title="Name: gray24 RGB: 61/61/61" style="background-color: rgb(61, 61, 61);" onclick="copyToClipboard('gray24', '61/61/61', notification)"></div>
            <div class="color-box" title="Name: gray25 RGB: 64/64/64" style="background-color: rgb(64, 64, 64);" onclick="copyToClipboard('gray25', '64/64/64', notification)"></div>
            <div class="color-box" title="Name: gray26 RGB: 66/66/66" style="background-color: rgb(66, 66, 66);" onclick="copyToClipboard('gray26', '66/66/66', notification)"></div>
            <div class="color-box" title="Name: gray27 RGB: 69/69/69" style="background-color: rgb(69, 69, 69);" onclick="copyToClipboard('gray27', '69/69/69', notification)"></div>
            <div class="color-box" title="Name: gray28 RGB: 71/71/71" style="background-color: rgb(71, 71, 71);" onclick="copyToClipboard('gray28', '71/71/71', notification)"></div>
            <div class="color-box" title="Name: gray29 RGB: 74/74/74" style="background-color: rgb(74, 74, 74);" onclick="copyToClipboard('gray29', '74/74/74', notification)"></div>
            <div class="color-box" title="Name: gray30 RGB: 77/77/77" style="background-color: rgb(77, 77, 77);" onclick="copyToClipboard('gray30', '77/77/77', notification)"></div>
            <div class="color-box" title="Name: gray31 RGB: 79/79/79" style="background-color: rgb(79, 79, 79);" onclick="copyToClipboard('gray31', '79/79/79', notification)"></div>
            <div class="color-box" title="Name: gray32 RGB: 82/82/82" style="background-color: rgb(82, 82, 82);" onclick="copyToClipboard('gray32', '82/82/82', notification)"></div>
            <div class="color-box" title="Name: gray33 RGB: 84/84/84" style="background-color: rgb(84, 84, 84);" onclick="copyToClipboard('gray33', '84/84/84', notification)"></div>
            <div class="color-box" title="Name: gray34 RGB: 87/87/87" style="background-color: rgb(87, 87, 87);" onclick="copyToClipboard('gray34', '87/87/87', notification)"></div>
            <div class="color-box" title="Name: gray35 RGB: 89/89/89" style="background-color: rgb(89, 89, 89);" onclick="copyToClipboard('gray35', '89/89/89', notification)"></div>
            <div class="color-box" title="Name: gray36 RGB: 92/92/92" style="background-color: rgb(92, 92, 92);" onclick="copyToClipboard('gray36', '92/92/92', notification)"></div>
            <div class="color-box" title="Name: gray37 RGB: 94/94/94" style="background-color: rgb(94, 94, 94);" onclick="copyToClipboard('gray37', '94/94/94', notification)"></div>
            <div class="color-box" title="Name: gray38 RGB: 97/97/97" style="background-color: rgb(97, 97, 97);" onclick="copyToClipboard('gray38', '97/97/97', notification)"></div>
            <div class="color-box" title="Name: gray39 RGB: 99/99/99" style="background-color: rgb(99, 99, 99);" onclick="copyToClipboard('gray39', '99/99/99', notification)"></div>
            <div class="color-box" title="Name: gray40 RGB: 102/102/102" style="background-color: rgb(102, 102, 102);" onclick="copyToClipboard('gray40', '102/102/102', notification)"></div>
            <div class="color-box" title="Name: gray41 RGB: 105/105/105" style="background-color: rgb(105, 105, 105);" onclick="copyToClipboard('gray41', '105/105/105', notification)"></div>
            <div class="color-box" title="Name: gray42 RGB: 107/107/107" style="background-color: rgb(107, 107, 107);" onclick="copyToClipboard('gray42', '107/107/107', notification)"></div>
            <div class="color-box" title="Name: gray43 RGB: 110/110/110" style="background-color: rgb(110, 110, 110);" onclick="copyToClipboard('gray43', '110/110/110', notification)"></div>
            <div class="color-box" title="Name: gray44 RGB: 112/112/112" style="background-color: rgb(112, 112, 112);" onclick="copyToClipboard('gray44', '112/112/112', notification)"></div>
            <div class="color-box" title="Name: gray45 RGB: 115/115/115" style="background-color: rgb(115, 115, 115);" onclick="copyToClipboard('gray45', '115/115/115', notification)"></div>
            <div class="color-box" title="Name: gray46 RGB: 117/117/117" style="background-color: rgb(117, 117, 117);" onclick="copyToClipboard('gray46', '117/117/117', notification)"></div>
            <div class="color-box" title="Name: gray47 RGB: 120/120/120" style="background-color: rgb(120, 120, 120);" onclick="copyToClipboard('gray47', '120/120/120', notification)"></div>
            <div class="color-box" title="Name: gray48 RGB: 122/122/122" style="background-color: rgb(122, 122, 122);" onclick="copyToClipboard('gray48', '122/122/122', notification)"></div>
            <div class="color-box" title="Name: gray49 RGB: 125/125/125" style="background-color: rgb(125, 125, 125);" onclick="copyToClipboard('gray49', '125/125/125', notification)"></div>
            <div class="color-box" title="Name: gray50 RGB: 127/127/127" style="background-color: rgb(127, 127, 127);" onclick="copyToClipboard('gray50', '127/127/127', notification)"></div>
            <div class="color-box" title="Name: gray51 RGB: 130/130/130" style="background-color: rgb(130, 130, 130);" onclick="copyToClipboard('gray51', '130/130/130', notification)"></div>
            <div class="color-box" title="Name: gray52 RGB: 133/133/133" style="background-color: rgb(133, 133, 133);" onclick="copyToClipboard('gray52', '133/133/133', notification)"></div>
            <div class="color-box" title="Name: gray53 RGB: 135/135/135" style="background-color: rgb(135, 135, 135);" onclick="copyToClipboard('gray53', '135/135/135', notification)"></div>
            <div class="color-box" title="Name: gray54 RGB: 138/138/138" style="background-color: rgb(138, 138, 138);" onclick="copyToClipboard('gray54', '138/138/138', notification)"></div>
            <div class="color-box" title="Name: gray55 RGB: 140/140/140" style="background-color: rgb(140, 140, 140);" onclick="copyToClipboard('gray55', '140/140/140', notification)"></div>
            <div class="color-box" title="Name: gray56 RGB: 143/143/143" style="background-color: rgb(143, 143, 143);" onclick="copyToClipboard('gray56', '143/143/143', notification)"></div>
            <div class="color-box" title="Name: gray57 RGB: 145/145/145" style="background-color: rgb(145, 145, 145);" onclick="copyToClipboard('gray57', '145/145/145', notification)"></div>
            <div class="color-box" title="Name: gray58 RGB: 148/148/148" style="background-color: rgb(148, 148, 148);" onclick="copyToClipboard('gray58', '148/148/148', notification)"></div>
            <div class="color-box" title="Name: gray59 RGB: 150/150/150" style="background-color: rgb(150, 150, 150);" onclick="copyToClipboard('gray59', '150/150/150', notification)"></div>
            <div class="color-box" title="Name: gray60 RGB: 153/153/153" style="background-color: rgb(153, 153, 153);" onclick="copyToClipboard('gray60', '153/153/153', notification)"></div>
            <div class="color-box" title="Name: gray61 RGB: 156/156/156" style="background-color: rgb(156, 156, 156);" onclick="copyToClipboard('gray61', '156/156/156', notification)"></div>
            <div class="color-box" title="Name: gray62 RGB: 158/158/158" style="background-color: rgb(158, 158, 158);" onclick="copyToClipboard('gray62', '158/158/158', notification)"></div>
            <div class="color-box" title="Name: gray63 RGB: 161/161/161" style="background-color: rgb(161, 161, 161);" onclick="copyToClipboard('gray63', '161/161/161', notification)"></div>
            <div class="color-box" title="Name: gray64 RGB: 163/163/163" style="background-color: rgb(163, 163, 163);" onclick="copyToClipboard('gray64', '163/163/163', notification)"></div>
            <div class="color-box" title="Name: gray65 RGB: 166/166/166" style="background-color: rgb(166, 166, 166);" onclick="copyToClipboard('gray65', '166/166/166', notification)"></div>
            <div class="color-box" title="Name: gray66 RGB: 168/168/168" style="background-color: rgb(168, 168, 168);" onclick="copyToClipboard('gray66', '168/168/168', notification)"></div>
            <div class="color-box" title="Name: gray67 RGB: 171/171/171" style="background-color: rgb(171, 171, 171);" onclick="copyToClipboard('gray67', '171/171/171', notification)"></div>
            <div class="color-box" title="Name: gray68 RGB: 173/173/173" style="background-color: rgb(173, 173, 173);" onclick="copyToClipboard('gray68', '173/173/173', notification)"></div>
            <div class="color-box" title="Name: gray69 RGB: 176/176/176" style="background-color: rgb(176, 176, 176);" onclick="copyToClipboard('gray69', '176/176/176', notification)"></div>
            <div class="color-box" title="Name: gray70 RGB: 179/179/179" style="background-color: rgb(179, 179, 179);" onclick="copyToClipboard('gray70', '179/179/179', notification)"></div>
            <div class="color-box" title="Name: gray71 RGB: 181/181/181" style="background-color: rgb(181, 181, 181);" onclick="copyToClipboard('gray71', '181/181/181', notification)"></div>
            <div class="color-box" title="Name: gray72 RGB: 184/184/184" style="background-color: rgb(184, 184, 184);" onclick="copyToClipboard('gray72', '184/184/184', notification)"></div>
            <div class="color-box" title="Name: gray73 RGB: 186/186/186" style="background-color: rgb(186, 186, 186);" onclick="copyToClipboard('gray73', '186/186/186', notification)"></div>
            <div class="color-box" title="Name: gray74 RGB: 189/189/189" style="background-color: rgb(189, 189, 189);" onclick="copyToClipboard('gray74', '189/189/189', notification)"></div>
            <div class="color-box" title="Name: gray75 RGB: 191/191/191" style="background-color: rgb(191, 191, 191);" onclick="copyToClipboard('gray75', '191/191/191', notification)"></div>
            <div class="color-box" title="Name: gray76 RGB: 194/194/194" style="background-color: rgb(194, 194, 194);" onclick="copyToClipboard('gray76', '194/194/194', notification)"></div>
            <div class="color-box" title="Name: gray77 RGB: 196/196/196" style="background-color: rgb(196, 196, 196);" onclick="copyToClipboard('gray77', '196/196/196', notification)"></div>
            <div class="color-box" title="Name: gray78 RGB: 199/199/199" style="background-color: rgb(199, 199, 199);" onclick="copyToClipboard('gray78', '199/199/199', notification)"></div>
            <div class="color-box" title="Name: gray79 RGB: 201/201/201" style="background-color: rgb(201, 201, 201);" onclick="copyToClipboard('gray79', '201/201/201', notification)"></div>
            <div class="color-box" title="Name: gray80 RGB: 204/204/204" style="background-color: rgb(204, 204, 204);" onclick="copyToClipboard('gray80', '204/204/204', notification)"></div>
            <div class="color-box" title="Name: gray81 RGB: 207/207/207" style="background-color: rgb(207, 207, 207);" onclick="copyToClipboard('gray81', '207/207/207', notification)"></div>
            <div class="color-box" title="Name: gray82 RGB: 209/209/209" style="background-color: rgb(209, 209, 209);" onclick="copyToClipboard('gray82', '209/209/209', notification)"></div>
            <div class="color-box" title="Name: gray83 RGB: 212/212/212" style="background-color: rgb(212, 212, 212);" onclick="copyToClipboard('gray83', '212/212/212', notification)"></div>
            <div class="color-box" title="Name: gray84 RGB: 214/214/214" style="background-color: rgb(214, 214, 214);" onclick="copyToClipboard('gray84', '214/214/214', notification)"></div>
            <div class="color-box" title="Name: gray85 RGB: 217/217/217" style="background-color: rgb(217, 217, 217);" onclick="copyToClipboard('gray85', '217/217/217', notification)"></div>
            <div class="color-box" title="Name: gray86 RGB: 219/219/219" style="background-color: rgb(219, 219, 219);" onclick="copyToClipboard('gray86', '219/219/219', notification)"></div>
            <div class="color-box" title="Name: gray87 RGB: 222/222/222" style="background-color: rgb(222, 222, 222);" onclick="copyToClipboard('gray87', '222/222/222', notification)"></div>
            <div class="color-box" title="Name: gray88 RGB: 224/224/224" style="background-color: rgb(224, 224, 224);" onclick="copyToClipboard('gray88', '224/224/224', notification)"></div>
            <div class="color-box" title="Name: gray89 RGB: 227/227/227" style="background-color: rgb(227, 227, 227);" onclick="copyToClipboard('gray89', '227/227/227', notification)"></div>
            <div class="color-box" title="Name: gray90 RGB: 229/229/229" style="background-color: rgb(229, 229, 229);" onclick="copyToClipboard('gray90', '229/229/229', notification)"></div>
            <div class="color-box" title="Name: gray91 RGB: 232/232/232" style="background-color: rgb(232, 232, 232);" onclick="copyToClipboard('gray91', '232/232/232', notification)"></div>
            <div class="color-box" title="Name: gray92 RGB: 235/235/235" style="background-color: rgb(235, 235, 235);" onclick="copyToClipboard('gray92', '235/235/235', notification)"></div>
            <div class="color-box" title="Name: gray93 RGB: 237/237/237" style="background-color: rgb(237, 237, 237);" onclick="copyToClipboard('gray93', '237/237/237', notification)"></div>
            <div class="color-box" title="Name: gray94 RGB: 240/240/240" style="background-color: rgb(240, 240, 240);" onclick="copyToClipboard('gray94', '240/240/240', notification)"></div>
            <div class="color-box" title="Name: gray95 RGB: 242/242/242" style="background-color: rgb(242, 242, 242);" onclick="copyToClipboard('gray95', '242/242/242', notification)"></div>
            <div class="color-box" title="Name: gray96 RGB: 245/245/245" style="background-color: rgb(245, 245, 245);" onclick="copyToClipboard('gray96', '245/245/245', notification)"></div>
            <div class="color-box" title="Name: gray97 RGB: 247/247/247" style="background-color: rgb(247, 247, 247);" onclick="copyToClipboard('gray97', '247/247/247', notification)"></div>
            <div class="color-box" title="Name: gray98 RGB: 250/250/250" style="background-color: rgb(250, 250, 250);" onclick="copyToClipboard('gray98', '250/250/250', notification)"></div>
            <div class="color-box" title="Name: gray99 RGB: 252/252/252" style="background-color: rgb(252, 252, 252);" onclick="copyToClipboard('gray99', '252/252/252', notification)"></div>
            <div class="color-box" title="Name: gray100 RGB: 255/255/255" style="background-color: rgb(255, 255, 255);" onclick="copyToClipboard('gray100', '255/255/255', notification)"></div>
        </div>
    </div>

    <div id="notification">RGB value copied to clipboard</div>

    <script>
        // Function to copy text to clipboard and show notification
        function copyToClipboard(name, rgb, notification) {
            const textToCopy = `Name: ${name}\nRGB: ${rgb}`;
            const textArea = document.createElement('textarea');
            textArea.value = textToCopy;
            document.body.appendChild(textArea);
            textArea.select();
            document.execCommand('copy');
            document.body.removeChild(textArea);

            // Show the notification
            notification.style.display = 'block';

            // Hide the notification after a delay (e.g., 2 seconds)
            setTimeout(() => {
                notification.style.display = 'none';
            }, 2000);
        }
    </script>