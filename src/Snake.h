#ifndef SNAKE_H
#define SNAKE_H

#include <WinSock2.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef BUILD_SERVER
#include "SnakeServer.h"
#else
#include "SnakeClient.h"
#include "ClientDialogs.h"
#endif // BUILD_SERVER

#endif // SNAKE_H
