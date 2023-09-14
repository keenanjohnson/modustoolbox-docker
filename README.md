# modustoolbox-docker
A dockerfile container Infineon's ModusToolbox Software.

## Features

This is a linux/amd64 docker image based on ubuntu:20.04.

It contains the [ModusToolBox Software from Infineon](https://softwaretools.infineon.com/tools/com.ifx.tb.tool.modustoolbox).

Current Version: 3.1.0.12257

You can inspect the contents of what will be installed in the docker container in the [ModusToolbox directory](ModusToolbox).

The enviroment variable: CY_TOOLS_PATHS is already set inside of the docker container and points to the install directory for Modustoolbox.

This means that if you set up a ModusToolbox project, you should be able to run ```make build``` to build the project.

## Usage

Pull this image with the following command:

```docker pull --platform=linux/amd64 ghcr.io/keenanjohnson/modustoolbox-docker:main```

or from within a Dockerfile:

```FROM --platform=linux/amd64 ghcr.io/keenanjohnson/modustoolbox-docker:main ghcr.io/keenanjohnson/modustoolbox-docker:main```

## VSCode Dev Containers

It also designed to be easily used inside of a [Visual Studio Code Dev Container](https://code.visualstudio.com/docs/devcontainers/containers) which I highly suggest.

The minimum files needed to get started there are a devcontainer.json and a Dockerfile in the .devcontainer directory of the project.

### devcontainer.json
```
// For format details, see https://aka.ms/devcontainer.json. For config options, see the
// README at: https://github.com/devcontainers/templates/tree/main/src/cpp
{
	"name": "ModusToolbox Dev Container",
	"build": {
		"dockerfile": "Dockerfile"
	}
}
```

### Dockerfile
```
FROM --platform=linux/amd64 ghcr.io/keenanjohnson/modustoolbox-docker:main
```
