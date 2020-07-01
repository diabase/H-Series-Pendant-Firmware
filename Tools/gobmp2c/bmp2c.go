package main

import (
	"fmt"
	"os"
	"strings"

	"golang.org/x/image/bmp"
)

func main() {
	for _, file := range os.Args[1:] {
		f, err := os.Open(file)
		if err != nil {
			panic(err)
		}
		defer f.Close()

		b, err := bmp.Decode(f)
		if err != nil {
			panic(err)
		}

		start := true
		fmt.Printf("extern const uint8_t %s[] =\n", strings.TrimSuffix(file, "_30h.bmp"))
		fmt.Printf("{\t%d, %d,\t// width, height\n", b.Bounds().Dx(), b.Bounds().Dy())
		for x := b.Bounds().Min.X; x < b.Bounds().Max.X; x++ {
			fmt.Print("\t")
			for y := b.Bounds().Min.Y; y < b.Bounds().Max.Y; y++ {
				c := b.At(x, y)
				if start {
					fmt.Print("0x")
				}
				fmt.Print(getPaletteIndex(c.RGBA()))
				if !start {
					fmt.Print(", ")
				}
				start = !start
			}
			fmt.Println()
		}
		fmt.Println("};")
	}
}

func getPaletteIndex(r, g, b, a uint32) int {
	if r == 0xffff && g == 0xffff && b == 0xffff {
		return 1
	} else if r == 0xfafa && g == 0xfafa && b == 0xfafa {
		return 3
	} else {
	}
	return 0
}
