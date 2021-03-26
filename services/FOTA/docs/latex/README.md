# LaTeX

This directory contains the [LaTeX](https://www.latex-project.org/) source for generating some of the images used in this service's documentation.

Protocol packet structure diagrams were generated using the [bytefield package](https://ctan.org/pkg/bytefield).

You can install this package by first downloading the .zip file from the package site above and extract it. Then, open the `bytefield.ins` in a LaTeX editor and compile it. This will create an installable `bytefield.sty` file. Copy this `.sty` file into a suitable location where LaTeX searches, eg: `~/texmf/latex/bytestream`


[Click here](https://en.wikibooks.org/wiki/LaTeX/Installing_Extra_Packages) to learn more about installing LaTeX packages.

To build the .tex files, use the following command:

`latex <file>.tex`

Then open the `.dvi` file in the appropriate document viewer.
