# Developer Manual:  Digital Clock 🕐

These are the LaTeX sources for the Developer Manual.

## Building 🔨

To compile the manual yourself, it's easiest to use [Visual Studio Code](https://code.visualstudio.com/), the [LaTeX Workshop](https://marketplace.visualstudio.com/items?itemName=James-Yu.latex-workshop) extension and [Docker](https://www.docker.com/). Inside Visual Studio Code open `File > Preferences > Settings` and search for `latex-workshop.docker.enabled`. Tick the option to **enable Docker builds**. Next search for `latex-workshop.docker.image.latex` and enter `gokhlayeh/latex`. This will instruct LaTeX to download the prepared [Docker image](https://hub.docker.com/repository/docker/gokhlayeh/latex), containing all required build tools and TeX Live modules (this may take some minutes on first build).

Afterwards open `main.tex` in vscode and compile it by hitting `CTRL + ALT + B` or look for the option `TeX > Build LaTeX project > Recipe: latexmk 🔁` in the side menubar.

Compiling can also be done outside of vscode. Open a shell in the `.../doc/` directory, and run:

```sh
docker pull gokhlayeh/latex
docker run -it --rm -v "$(pwd):/workspace" -w '/workspace' gokhlayeh/latex latexmk -out-dir=build
```
