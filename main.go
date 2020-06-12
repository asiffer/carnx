package main

import (
	"carnx/api"
	"os"

	cli "github.com/urfave/cli/v2"
)

var app = &cli.App{
	Name:   "carnxd",
	Usage:  "XDP based network counter digger",
	Action: run,
	Flags: []cli.Flag{
		&cli.BoolFlag{
			Name:    "debug",
			Aliases: []string{"v"},
			Value:   false,
			Usage:   "Activate debugging mode",
		},
		&cli.StringFlag{
			Name:    "load",
			Aliases: []string{"l"},
			Usage:   "BPF program to load",
		},
		&cli.StringFlag{
			Name:    "interface",
			Aliases: []string{"i"},
			Usage:   "Interface to attach the program",
		},
		&cli.UintFlag{
			Name:    "xdp-flags",
			Value:   0,
			Aliases: []string{"x"},
			Usage:   "XDP flags (set the way to attach the program to the interface)",
		},
		&cli.StringFlag{
			Name:    "unix",
			Value:   "/run/carnx.sock",
			Aliases: []string{"u"},
			Usage:   "Listening unix socket",
		},
		&cli.BoolFlag{
			Name:    "systemd",
			Aliases: []string{"s"},
			Usage:   "Use a systemd socket (see carnx.socket)",
		},
	},
}

func run(c *cli.Context) error {

	s := &CarnxServer{}
	if c.Bool("debug") {
		setDebug()
	}

	// loading
	if prog := c.String("load"); prog != "" {
		_, err := s.Load(nil, &api.LoadParameters{BpfProgram: prog})
		if err != nil {
			return err
		}

		// attaching
		if iface := c.String("interface"); iface != "" {
			_, err := s.Attach(nil,
				&api.AttachParameters{Interface: iface,
					XdpFlags: uint32(c.Uint("xdp-flags"))})
			if err != nil {
				return err
			}
		}
	}

	// use the systemd socket
	if c.Bool("systemd") {
		return ServeSystemd()
	}

	// serve unix endpoint by default
	unixPath := c.String("unix")
	if err := ServeUnix(unixPath); err != nil {
		errorf("%v", err)
	}
	if err := removeFileIfExists(unixPath); err != nil {
		errorf("%v", err)
	}
	return nil
}

func main() {
	app.Run(os.Args)
}
