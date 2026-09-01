# Usa l'immagine ufficiale completa di TeXLive
FROM texlive/texlive:latest

# Aggiorna il sistema e installa Python, Pygments e Git (utile per VS Code)
RUN apt-get update && apt-get install -y \
	python3 \
	python3-pygments \
	git \
	&& rm -rf /var/lib/apt/lists/*
