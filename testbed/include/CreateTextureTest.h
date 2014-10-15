/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef CREATETEXTURETEST_H__
#define CREATETEXTURETEST_H__


class CreateTextureTest : public Heart::hStateBase
{
public:
	CreateTextureTest( Heart::hHeartEngine* engine ) 
		: hStateBase( "SimpleRoomAddedLight" )
		,engine_( engine )
	{

	}
	~CreateTextureTest() {}

	virtual void				PreEnter() {}
	virtual hUint32				Enter() { return Heart::hStateBase::FINISHED; }
	virtual void				PostEnter();
	virtual hUint32				Main();		
	virtual void				MainRender();
	virtual void				PreLeave() {}
	virtual hUint32				Leave();

private:

	Heart::hHeartEngine*						engine_;
};

#endif // CREATETEXTURETEST_H__