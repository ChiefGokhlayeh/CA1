# Developer Manual: Radio Controlled Digital Clock 📶 🕐

These are the LaTeX sources for the Developer Manual.

## Building 🔨

To compile the manual yourself, it's easiest to use [Visual Studio Code](https://code.visualstudio.com/), the [LaTeX Workshop](https://marketplace.visualstudio.com/items?itemName=James-Yu.latex-workshop) extension and [Docker](https://www.docker.com/). Inside Visual Studio Code open `File > Preferences > Settings` and search for `latex-workshop.docker.enabled`. Tick the option to **enable Docker builds**. Next search for `latex-workshop.docker.image.latex` and enter `gokhlayeh/latex`. This will instruct LaTeX to download the prepared [Docker image](https://hub.docker.com/repository/docker/gokhlayeh/latex), containing all required build tools and TeX Live modules (this may take some minutes on first build). Pulling the image manually might provide better progress feedback, than what LaTeX Workshop gives you. To pull, simply run:

```sh
docker pull gokhlayeh/latex
```

Before we can build our final document however, we have to render some [PlantUML](https://plantuml.com/) diagrams into image files. There is a `Makefile` provided to do just that. If you have PlantUML installed natively, you can just run:

```sh
make images
```

Otherwise, PlantUML is of course also included in the Docker build image.

```sh
docker run -it --rm -v "$(pwd):/workspace" -w '/workspace' gokhlayeh/latex make images
```

Afterwards open `main.tex` in vscode and compile it by hitting `CTRL + ALT + B` or look for the option `TeX > Build LaTeX project > Recipe: latexmk 🔃` in the side menubar. To ensure LaTeX Workshop always compiles with the latest PlantUML images, it's recommended to change the way LaTeX Workshop attempts to build the document. Create a `.vscode/settings.json`, and paste the following configuration into it:

```jsonc
{
    "latex-workshop.latex.tools": [
        {
            "name": "latexmk",
            "command": "latexmk",
            "args": [
                "-synctex=1",
                "-interaction=nonstopmode",
                "-file-line-error",
                "-pdf",
                "-outdir=%OUTDIR%",
                "%DOC%"
            ],
            "env": {}
        },
        {
            "name": "make images",
            "command": "make",
            "args": [
                "images"
            ]
        }
    ],
    "latex-workshop.latex.recipes": [
        {
            "name": "latexmk 🔃",
            "tools": [
                "make images", // This will check if images need to be rendered before generating the PDF
                "latexmk"
            ]
        },
    ],
}
```

Compiling can of course also be done outside of vscode. Open a shell in the `.../doc/` directory, and run:

```sh
docker run -it --rm -v "$(pwd):/workspace" -w '/workspace' gokhlayeh/latex make
```
