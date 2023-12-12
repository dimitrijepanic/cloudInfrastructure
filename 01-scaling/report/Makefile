IMGPATH           = img/
IMG_GEN_PATH      = $(IMGPATH)generated/
DATAPATH          = data/
DATA_PAR_PATH     = $(DATAPATH)parallel_calls/
DATA_SU_PATH      = $(DATAPATH)scaled_up/
PLOTS_SINGLE      = ../client/plot_single.py
PLOTS_MULT        = ../client/plot_multiples.py
DATA              = $(wildcard $(DATAPATH)*.dat)
SINGLE_IMG        = $(DATA:$(DATAPATH)%.dat=$(IMG_GEN_PATH)%.png)
PLOTSIZE          = 100
PYTHON            = python

IMGS = \
	$(IMGPATH)dockerhub.png \
	$(IMG_GEN_PATH)timedata_single.png \
	$(IMG_GEN_PATH)parallel_fig.png \
	$(IMG_GEN_PATH)scaled_up.png \

.PHONY: all clean mrproper

all: report.pdf

$(SINGLE_IMG): $(IMG_GEN_PATH)%.png: $(DATAPATH)%.dat $(PLOTS_SINGLE)
	mkdir -p $(IMG_GEN_PATH)
	$(PYTHON) $(PLOTS_SINGLE) $(PLOTSIZE) $< $@

$(IMG_GEN_PATH)parallel_fig.png: $(wildcard $(DATA_PAR_PATH)*.dat) $(PLOTS_MULT)
	mkdir -p $(IMG_GEN_PATH)
	$(PYTHON) $(PLOTS_MULT) $(PLOTSIZE) $(DATA_PAR_PATH) $@

$(IMG_GEN_PATH)scaled_up.png: $(wildcard $(DATA_SU_PATH)*.dat) $(PLOTS_MULT)
	mkdir -p $(IMG_GEN_PATH)
	$(PYTHON) $(PLOTS_MULT) $(PLOTSIZE) $(DATA_SU_PATH) $@


report.pdf: report.md $(IMGS)
	pandoc $< -f markdown -o $@

clean:
	rm -rfv *.aux *.bbl *.blg *.log *.out *.toc $(IMG_GEN_PATH)

mrproper: clean
	rm -fv *.pdf