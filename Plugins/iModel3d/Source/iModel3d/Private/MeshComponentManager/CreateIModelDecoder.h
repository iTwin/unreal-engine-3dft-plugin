#pragma once

#include "CoreMinimal.h"

#include "IModelDecoder.h"

#include <memory>

std::shared_ptr<IModelDecoder> CreateIModelDecoder();
