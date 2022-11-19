# ECGL (Embedded Canvas Graphics Library)

### ECGL is a C language graphics library based on ``Javascript Canvas API syntax`` and is designed for embedded devices.

### 🛠️ This project is under construction.

### Current Process:

* Refactor shader feature
* Refactor fill and stroke function
* Add linecap support, including miter join, round join and bevel join. (This will be finished less than 1 month.)

#### If you are interested in this project, you can go to the branch ``core-early-build`` for details.

#### You can also contact me by

Email: ``killbugs@yeah.net``   Twitter: ``@ChenZihanQ``

## Current Canvas API Syntax Support

|    Canvas API    | Support |
| :--------------: | :-----: |
|    beginPath    |   ✅   |
|      moveTo      |   ✅   |
|      lineTo      |   ✅   |
|       fill       |   ✅   |
|      stroke      |   ✅   |
|     fillRect     |   ✅   |
|    strokeRect    |   ✅   |
|   roundedRect   |   ✅   |
|       arc       |   ✅   |
|    closePath    |   ✅   |
|     fillText     |  🛠️  |
|    drawImage    |  🛠️  |
| quadraticCurveTo |  🛠️  |
|  bezierCurveTo  |  🛠️  |
|       save       |  🛠️  |
|     restore     |  🛠️  |
|    translate    |  ⚠️  |
|      rotate      |  ⚠️  |
|      scale      |  ⚠️  |
|    transform    |  ⚠️  |
|       clip       |  ⚠️  |

``✅ Support | 🛠️ Under construction | ⚠️ Support in future``

## Other features

* Supports emulation on Windows, Linux and Mac is possible. (By GLFW library)
* Antialiasing grahpics available.
* Quick flood fill available.
* Pixel display format RGB565, RGB888 and RGBA32 support.
* Display devices ST7789 support. (Will add more in the future)
