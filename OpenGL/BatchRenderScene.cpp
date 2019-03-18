#include "BatchRenderScene.h"
#include <time.h>


BatchRenderScene::BatchRenderScene()
	:mNumberofSprites(1000000),
	mScaleMat(1.0f, 1.0f), 
	mRotMat(0.0f),
	mTranMat(0.0f, 0.0f),
	mOrtho(CreateOrthoMatrix(0.0f, 1024, 768, 0))
{
	mBatchRenderer = std::make_unique<BatchRenderer>(mNumberofSprites, 6, 4);

	mSprites.reserve(mNumberofSprites * 4);

	srand(time(NULL));

	float sizeSquare = sqrt((1024.0f * 768.0f)/mNumberofSprites); 
	unsigned int line = 1024 / sizeSquare;//number of square in one line
	unsigned int Column = 768 / sizeSquare; //number of square in one column
	Vector<float, 4> Color;
	float x = sizeSquare/2.0f;
	float y = sizeSquare/2.0f;
	//create buffer of sprites
	for (unsigned int i = 0; i < Column; i++)
	{
		for (unsigned int j = 0; j < line; j++)
		{
			Color = { rand() % 100 / 100.0f,  rand() % 100 / 100.0f,  rand() % 100 / 100.0f ,  1.0 };
			Sprite sprite(x, y, sizeSquare / 2, sizeSquare / 2, Color);
			VertexData2C* aux = static_cast<VertexData2C*>(sprite.GetData());

			mSprites.push_back(*aux);
			aux++;
			mSprites.push_back(*aux);
			aux++;
			mSprites.push_back(*aux);
			aux++;
			mSprites.push_back(*aux);
			
			x += (sizeSquare * 1.2f);
		}
		y += (sizeSquare * 1.2f);
		x = sizeSquare / 2.0f;
	}

	mShader = std::make_unique<Shader>("Color.shader");
	mShader->bind();

	Matrix<float, 3, 3> WorldTransform = mScaleMat * mRotMat * mTranMat;
	mMVP = WorldTransform * mOrtho; //Model view projection

	mU_MVP = mShader->GetUniformLocation("u_MVP");
	mShader->SetUniformMatrix3f(mU_MVP, mMVP);

	mBatchRenderer->begin();
	mBatchRenderer->add(mSprites);
	mBatchRenderer->end();


	mShader->unbind();
}

BatchRenderScene::~BatchRenderScene()
{
}

void BatchRenderScene::ImGuiRenderer()
{
}

void BatchRenderScene::Update()
{
	mSprites.clear();
	{
		float sizeSquare = sqrt((1024.0f * 768.0f) / mNumberofSprites);
		unsigned int line = 1024 / sizeSquare;//number of square in one line
		unsigned int Column = 768 / sizeSquare; //number of square in one column
		Vector<float, 4> Color;
		float x = sizeSquare / 2.0f;
		float y = sizeSquare / 2.0f;
		//create buffer of sprites
		for (unsigned int i = 0; i < Column; i++)
		{
			for (unsigned int j = 0; j < line; j++)
			{
				Color = { rand() % 100 / 100.0f,  rand() % 100 / 100.0f,  rand() % 100 / 100.0f ,  1.0 };
				Sprite sprite(x, y, sizeSquare / 2, sizeSquare / 2, Color);
				VertexData2C* aux = static_cast<VertexData2C*>(sprite.GetData());

				mSprites.push_back(*aux);
				aux++;
				mSprites.push_back(*aux);
				aux++;
				mSprites.push_back(*aux);
				aux++;
				mSprites.push_back(*aux);

				x += (sizeSquare * 1.2f);
			}
			y += (sizeSquare * 1.2f);
			x = sizeSquare / 2.0f;
		}
	}

	mBatchRenderer->begin();
	mBatchRenderer->add(mSprites);
	mBatchRenderer->end();

	mShader->bind();
	mShader->SetUniformMatrix3f(mU_MVP, mMVP);

	mBatchRenderer->Render(*mShader);
}
