import { useEffect, useState } from "react"

 function Checkout({
  cart,
  currentUser,
  onClose,
  onPlaceOrder,
}) {
  const [address, setAddress] = useState(
  () => localStorage.getItem("selectedAddress") || ""
)
  const [paymentMethod, setPaymentMethod] = useState("Cash on Delivery")

  const [couponCode, setCouponCode] = useState("")
const [appliedCoupon, setAppliedCoupon] = useState(null)
const [couponLoading, setCouponLoading] = useState(false)
const [couponError, setCouponError] = useState("")
const [availableCoupons, setAvailableCoupons] = useState([])
const [couponsLoading, setCouponsLoading] = useState(false)

  const [loading, setLoading] = useState(false)
  const [error, setError] = useState("")

  useEffect(() => {
  if (currentUser) {
    loadAvailableCoupons()
  }
}, [currentUser])

  const subtotal = cart.reduce(
  (sum, item) => sum + item.price * item.quantity,
  0
)

const discount = appliedCoupon
  ? appliedCoupon.discount
  : 0

const total = Math.max(
  0,
  subtotal - discount
)

const handleApplyCoupon = async () => {
  setCouponError("")

  if (!couponCode.trim()) {
    setCouponError("Please enter a coupon code.")
    return
  }

  if (subtotal <= 0) {
    setCouponError("Your cart is empty.")
    return
  }

  setCouponLoading(true)

  try {
    const token = localStorage.getItem("authToken")

    const response = await fetch(
      "http://localhost:18080/api/coupons/apply",
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          "Authorization": `Bearer ${token}`,
        },
        body: JSON.stringify({
          code: couponCode.trim(),
          subtotal,
        }),
      }
    )

    const data = await response.json()

    if (!response.ok) {
      throw new Error(
        data.message || "Unable to apply coupon"
      )
    }

    setAppliedCoupon(data)
    setCouponError("")
  } catch (error) {
    setAppliedCoupon(null)
    setCouponError(error.message)
  } finally {
    setCouponLoading(false)
  }
}

const loadAvailableCoupons = async () => {
  try {
    setCouponsLoading(true)

    const token = localStorage.getItem("authToken")

    const response = await fetch(
      "http://localhost:18080/api/coupons",
      {
        headers: {
          "Authorization": `Bearer ${token}`,
        },
      }
    )

    const data = await response.json()

    if (!response.ok) {
      throw new Error(
        data.message || "Unable to load coupons"
      )
    }

    setAvailableCoupons(data)
  } catch (error) {
    console.error("Failed to load coupons:", error)
  } finally {
    setCouponsLoading(false)
  }
}

  const handleSubmit = async (event) => {
    event.preventDefault()

    setError("")

    if (!currentUser) {
      setError("Please login before placing an order.")
      return
    }

    if (!address.trim()) {
      setError("Please enter your delivery address.")
      return
    }

    if (cart.length === 0) {
      setError("Your cart is empty.")
      return
    }

    setLoading(true)

    try {
      const response = await fetch(
        "http://localhost:18080/api/orders",
        {
          method: "POST",
          headers: {
  "Content-Type": "application/json",
  "Authorization": `Bearer ${localStorage.getItem("authToken")}`,
},
          body: JSON.stringify({
            userId: currentUser.id,
            address: address.trim(),
            paymentMethod,
            couponCode: appliedCoupon
  ? appliedCoupon.couponCode
  : "",
items: cart.map((item) => ({
              id: item.id,
              name: item.name,
              price: item.price,
              quantity: item.quantity,
            })),
          }),
        }
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Failed to place order"
        )
      }

      onPlaceOrder(data.order)

    } catch (error) {
      setError(error.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

      <div className="bg-white rounded-2xl shadow-xl w-full max-w-2xl p-8 max-h-[90vh] overflow-y-auto">

        <div className="flex justify-between items-center mb-6">

          <h2 className="text-3xl font-bold text-gray-800">
            Checkout
          </h2>

          <button
            onClick={onClose}
            className="text-gray-500 text-2xl hover:text-gray-800"
          >
            ×
          </button>

        </div>

        <form onSubmit={handleSubmit}>

          <h3 className="text-xl font-bold text-gray-800 mb-4">
            Delivery Address
          </h3>

          <textarea
            value={address}
            onChange={(event) => {
  setAddress(event.target.value)
  localStorage.setItem("selectedAddress", event.target.value)
}}
            placeholder="Enter your complete delivery address"
            rows="3"
            className="w-full border rounded-lg px-4 py-3 mb-6"
          />

          <h3 className="text-xl font-bold text-gray-800 mb-4">
  Coupon / Discount
</h3>

{couponsLoading ? (
  <p className="text-gray-500 mb-4">
    Loading available coupons...
  </p>
) : availableCoupons.length > 0 ? (
  <div className="space-y-3 mb-5">

    {availableCoupons.map((coupon) => (
      <div
        key={coupon.id}
        className="border border-orange-200 bg-orange-50 rounded-xl p-4"
      >

        <div className="flex justify-between items-start gap-4">

          <div>
            <p className="font-bold text-orange-700 text-lg">
              🎟️ {coupon.code}
            </p>

            <p className="text-gray-700 mt-1">
              Get{" "}
              <strong>
                {coupon.discountType === "percentage"
                  ? `${coupon.discountValue}% OFF`
                  : `₹${coupon.discountValue} OFF`}
              </strong>
            </p>

            <p className="text-sm text-gray-500 mt-1">
              On orders above ₹{coupon.minimumOrder}
            </p>
          </div>

          <button
            type="button"
            disabled={
              !!appliedCoupon ||
              subtotal < coupon.minimumOrder
            }
            onClick={() => {
              setCouponCode(coupon.code)
              setCouponError("")
            }}
            className="bg-orange-600 text-white px-4 py-2 rounded-lg
                       font-semibold hover:bg-orange-700
                       disabled:bg-gray-400 disabled:cursor-not-allowed"
          >
            {subtotal < coupon.minimumOrder
              ? "Not Eligible"
              : appliedCoupon
                ? "Applied"
                : "Use Coupon"}
          </button>

        </div>

      </div>
    ))}

  </div>
) : (
  <p className="text-gray-500 mb-4">
    No active coupons available.
  </p>
)}

<div className="flex gap-3 mb-3">

  <input
    type="text"
    value={couponCode}
    onChange={(event) =>
      setCouponCode(event.target.value.toUpperCase())
    }
    placeholder="Enter coupon code"
    className="flex-1 border rounded-lg px-4 py-3"
    disabled={!!appliedCoupon}
  />

  <button
    type="button"
    onClick={handleApplyCoupon}
    disabled={couponLoading || !!appliedCoupon}
    className="bg-orange-600 text-white px-5 py-3 rounded-lg
               font-semibold hover:bg-orange-700
               disabled:bg-gray-400"
  >
    {couponLoading
      ? "Applying..."
      : appliedCoupon
        ? "Applied"
        : "Apply"}
  </button>

</div>

{appliedCoupon && (
  <div className="bg-green-100 text-green-700 px-4 py-3 rounded-lg mb-6">
    Coupon <strong>{appliedCoupon.couponCode}</strong> applied.
    You saved ₹{appliedCoupon.discount}.
  </div>
)}

{couponError && (
  <div className="bg-red-100 text-red-700 px-4 py-3 rounded-lg mb-6">
    {couponError}
  </div>
)}

          <h3 className="text-xl font-bold text-gray-800 mb-4">
            Payment Method
          </h3>

          <div className="grid grid-cols-2 gap-3 mb-6">

            {[
              "Cash on Delivery",
              "Card",
              "UPI",
              "Wallet",
            ].map((method) => (

              <label
                key={method}
                className={`border rounded-lg p-4 cursor-pointer ${
                  paymentMethod === method
                    ? "border-orange-500 bg-orange-50"
                    : "border-gray-300"
                }`}
              >

                <input
                  type="radio"
                  name="payment"
                  value={method}
                  checked={paymentMethod === method}
                  onChange={(event) =>
                    setPaymentMethod(event.target.value)
                  }
                  className="mr-2"
                />

                {method}

              </label>

            ))}

          </div>

          <h3 className="text-xl font-bold text-gray-800 mb-4">
            Order Summary
          </h3>

          <div className="space-y-3">

            {cart.map((item) => (

              <div
                key={item.id}
                className="flex justify-between border-b pb-2"
              >

                <span>
                  {item.name} × {item.quantity}
                </span>

                <span className="font-semibold">
                  ₹{item.price * item.quantity}
                </span>

              </div>

            ))}

          </div>

          <div className="border-t mt-6 pt-5 space-y-3">

  <div className="flex justify-between text-gray-600">
    <span>Subtotal</span>
    <span>₹{subtotal}</span>
  </div>

  {discount > 0 && (
    <div className="flex justify-between text-green-600">
      <span>Discount</span>
      <span>-₹{discount}</span>
    </div>
  )}

  <div className="flex justify-between text-2xl font-bold">

    <span>Total</span>

    <span className="text-orange-600">
      ₹{total}
    </span>

  </div>

</div>

          {error && (
            <div className="bg-red-100 text-red-700 px-4 py-3 rounded-lg mt-5">
              {error}
            </div>
          )}

          <button
            type="submit"
            disabled={loading}
            className="w-full bg-orange-600 text-white py-4 rounded-lg
                       font-bold mt-6 hover:bg-orange-700
                       disabled:bg-gray-400"
          >
            {loading ? "Placing Order..." : "Place Order"}
          </button>

        </form>

      </div>

    </div>
  )
}

export default Checkout