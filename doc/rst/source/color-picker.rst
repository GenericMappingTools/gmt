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
        <!-- JavaScript will populate this div with color rows -->
    </div>

    <div id="notification">RGB value copied to clipboard</div>

    <script>
        // Function to create color boxes
        function createColorBoxes(data) {
            const colorContainer = document.getElementById('color-container');
            const notification = document.getElementById('notification');

            let colorRow = document.createElement('div');
            colorRow.className = 'color-row';

            data.forEach((color, index) => {
                const colorBox = document.createElement('div');
                colorBox.className = 'color-box';
                colorBox.style.backgroundColor = `rgb(${color.rgb})`;
                colorBox.title = `Name: ${color.name}\nRGB: ${color.rgb.replace(/,/g, '/')}`; // Use slash-separated RGB in tooltip
                colorBox.onclick = () => copyToClipboard(color.name, color.rgb, notification);

                colorRow.appendChild(colorBox);

                // Start a new row after every 26 color boxes
                if ((index + 1) % 26 === 0) {
                    colorContainer.appendChild(colorRow);
                    colorRow = document.createElement('div');
                    colorRow.className = 'color-row';
                }
            });

            // Add any remaining colors in the last row
            colorContainer.appendChild(colorRow);
        }

        // Function to copy text to clipboard and show notification
        function copyToClipboard(name, rgb, notification) {
            const textToCopy = `Name: ${name}\nRGB: ${rgb.replace(/,/g, '/')}`; // Replace commas with slashes
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

        // Load the JSON data from the file
        fetch('_static/gmt_colors.json')
            .then(response => response.json())
            .then(data => createColorBoxes(data))
            .catch(error => {
                console.error('Error loading JSON data:', error);
            });
    </script>
