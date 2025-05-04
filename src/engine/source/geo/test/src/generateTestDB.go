package main

import (
	"fmt"
	"net"
	"os"

	"github.com/maxmind/mmdbwriter"
	"github.com/maxmind/mmdbwriter/mmdbtype"
)

func main() {
	writer, err := mmdbwriter.New(mmdbwriter.Options{
		DatabaseType: "My-IP-Data",
		Description: map[string]string{
			"en": "Test database",
			"es": "Base de datos de prueba",
		},
		IPVersion:  4,
		RecordSize: 24,
	})
	if err != nil {
		fmt.Println("Error creating writer:", err)
		return
	}

	insert := func(cidr string, data mmdbtype.Map) {
		_, network, err := net.ParseCIDR(cidr)
		if err != nil {
			fmt.Printf("Invalid CIDR %s: %v\n", cidr, err)
			return
		}
		if err := writer.Insert(network, data); err != nil {
			fmt.Printf("Failed to insert data for %s: %v\n", cidr, err)
		}
	}

	insert("1.2.3.4/32", mmdbtype.Map{
		"test_map": mmdbtype.Map{
			"test_str1": mmdbtype.String("DistroDefender"),
			"test_str2": mmdbtype.String("DistroDefender2"),
		},
		"test_uint32":  mmdbtype.Uint32(94043),
		"test_double":  mmdbtype.Float64(37.386),
		"test_float":   mmdbtype.Float32(122.0838),
		"test_bytes":   mmdbtype.Bytes([]byte{0xab, 0xcd}),
		"test_uint16":  mmdbtype.Uint16(123),
		"test_uint64":  mmdbtype.Uint64(1234567890),
		"test_uint128": mmdbtype.String("0x0000000000000000ab54a98ceb1f0ad2"),
		"test_boolean": mmdbtype.Bool(true),
		"test_array": mmdbtype.Slice{
			mmdbtype.String("a"),
			mmdbtype.String("b"),
			mmdbtype.String("c"),
		},
	})

	insert("1.2.3.5/32", mmdbtype.Map{
		"test_map": mmdbtype.Map{
			"test_str1": mmdbtype.String("Missing values"),
		},
	})

	out, err := os.Create("testdb.mmdb")
	if err != nil {
		fmt.Println("Failed to create output file:", err)
		return
	}
	defer out.Close()

	if _, err := writer.WriteTo(out); err != nil {
		fmt.Println("Failed to write MMDB:", err)
		return
	}

	fmt.Println("testdb.mmdb has now been created")
}

