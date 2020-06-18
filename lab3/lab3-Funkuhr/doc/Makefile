ROOT_PATH = $(CURDIR)

GENERATED_IMAGES_DIR := images/generated
DIAGRAMS_DIR := diagrams
BUILD_DIR := build
CHAPTERS_DIR := chapters

DIAGRAMS := $(wildcard $(DIAGRAMS_DIR)/*.puml)
PNGS := $(DIAGRAMS:$(DIAGRAMS_DIR)/%.puml=$(GENERATED_IMAGES_DIR)/%.png)
SVGS := $(DIAGRAMS:$(DIAGRAMS_DIR)/%.puml=$(GENERATED_IMAGES_DIR)/%.svg)
PDF_TEXS := $(DIAGRAMS:$(DIAGRAMS_DIR)/%.puml=$(GENERATED_IMAGES_DIR)/%.pdf_tex)

LATEX := main.tex
CHAPTERS := $(wildcard $(CHAPTERS_DIR)/*.tex)
PDFS := $(LATEX:%.tex=$(BUILD_DIR)/%.pdf)

.PHONY: images pdf clean
all: pdf
images: $(SVGS) $(PDF_TEXS) #$(PNGS)
pdf: $(PDFS)
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(GENERATED_IMAGES_DIR)

$(PDFS): images $(CHAPTERS)

$(GENERATED_IMAGES_DIR)/%.png: $(DIAGRAMS_DIR)/%.puml
	@mkdir -p "$(ROOT_PATH)/$(dir $@)"
	plantuml -tpng -o "$(ROOT_PATH)/$(dir $@)" "$^"

$(GENERATED_IMAGES_DIR)/%.svg: $(DIAGRAMS_DIR)/%.puml
	@mkdir -p "$(ROOT_PATH)/$(dir $@)"
	plantuml -tsvg -o "$(ROOT_PATH)/$(dir $@)" "$^"

$(GENERATED_IMAGES_DIR)/%.pdf_tex: $(GENERATED_IMAGES_DIR)/%.svg
	@mkdir -p "$(ROOT_PATH)/$(dir $@)"
	inkscape --export-type=pdf --export-latex "$^"

$(BUILD_DIR)/$(CHAPTERS_DIR):
	@mkdir -p "$(ROOT_PATH)/$@"

$(BUILD_DIR)/%.pdf: $(ROOT_PATH)/%.tex $(BUILD_DIR)/$(CHAPTERS_DIR)
	@mkdir -p "$(ROOT_PATH)/$(dir $@)"
	@latexmk -interaction=nonstopmode -file-line-error -pdf -outdir="$(ROOT_PATH)/$(dir $@)"
