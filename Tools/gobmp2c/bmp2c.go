package main

import (
	"encoding/binary"
	"flag"
	"fmt"
	"math"
	"os"
	"strings"

	"golang.org/x/image/bmp"
)

func main() {
	var binaryOutput bool
	flag.BoolVar(&binaryOutput, "binary", false, "Binary output")
	flag.Parse()

	files := flag.Args()

	for _, file := range files {
		f, err := os.Open(file)
		if err != nil {
			panic(err)
		}
		defer f.Close()

		b, err := bmp.Decode(f)
		if err != nil {
			panic(err)
		}

		if binaryOutput {
			writeBinary(uint16(b.Bounds().Max.X))
			writeBinary(uint16(b.Bounds().Max.Y))

			repeatCount := uint32(0)
			lastData := uint16(0)
			for y := b.Bounds().Max.Y; y >= b.Bounds().Min.Y; {
				y--
				for x := b.Bounds().Min.X; x < b.Bounds().Max.X; x++ {
					c := b.At(x, y)
					oc := convertTo16BitColor(c.RGBA())
					if repeatCount > 0 && (lastData != oc || repeatCount == math.MaxUint16) {
						writeBinary(repeatCount - 1)
						writeBinary(lastData)
					}
					lastData = oc
					repeatCount++
					// Last pixel
					if x+1 == b.Bounds().Max.X && y == 0 {
						writeBinary(repeatCount - 1)
						writeBinary(lastData)
					}
				}
			}
		} else {
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

func convertTo16BitColor(r, g, b, a uint32) uint16 {
	result := uint16(0)
	result |= uint16((r & 0xF8) << 8)
	result |= uint16((g & 0xFC) << 3)
	result |= uint16((b & 0xF8) >> 3)
	return result
}

func writeBinary(data interface{}) error {
	return binary.Write(os.Stdout, binary.LittleEndian, data)
}
